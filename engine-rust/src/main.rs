mod rule;

use std::env;
use rand::rngs::SmallRng;
use rand_distr::{Distribution, Beta};
use redb::{Database, ReadOnlyTable, ReadableDatabase, ReadableTableMetadata, TableDefinition};

const NODES: TableDefinition<(u64, u64), f32> = TableDefinition::new("nodes");

pub struct Node {
    pub mine:u64,
    pub oppo: u64,
    pub a: f32,
    pub b: f32,
    pub children: Vec<Node>
}

impl Node {

    pub fn new(mine: u64, oppo: u64) -> Self {
        Node {
            mine: mine,
            oppo: oppo,
            a: 1.0,
            b: 1.0,
            children: Vec::new(),
        }
    }

    fn update_param(self: &mut Self, result: &mut GameResult) {
        match result {
            GameResult::Win => {
                self.a += 1.0;
                *result = GameResult::Lose;
            }
            GameResult::Lose => {
                self.b += 1.0;
                *result = GameResult::Win;
            }
            GameResult::Draw => {
                self.a += 0.5;
                self.b += 0.5;
                *result = GameResult::Draw;
            }
            GameResult::None => panic!("不正なゲーム結果"),
        }
    }

    fn make_children(self: &mut Self, legals: &[u64; 8], table: &redb::ReadOnlyTable<(u64, u64), f32>) {
        let mut movable: u64 = rule::get_legal(legals);
        let mut lsb: u64;
        let mut reversable: u64;
        let mut child: Node;
        while movable > 0 {
            lsb = movable & movable.wrapping_neg();
            reversable = rule::get_reversable(self.oppo, lsb, legals);
            child = Node::new(self.oppo ^ reversable, self.mine | lsb | reversable);
            read_hashmap(&mut child, table);
            self.children.push(child);
            movable &= movable - 1;
        }
    }

}

pub enum GameResult {
    Win,
    Lose,
    Draw,
    None,
}

fn end_game(node: &Node, result: &mut GameResult) {
    let count_m: u32 = node.mine.count_ones();
    let count_y: u32 = node.oppo.count_ones();
    if count_m > count_y {
        *result = GameResult::Win;
    } else if count_m < count_y {
        *result = GameResult::Lose;
    } else {
        *result = GameResult::Draw;
    }
}

fn move_child(node: &Node) -> usize {
    let mut rng: SmallRng = rand::make_rng();
    let mut sample: f32;
    let mut min_score: f32 = 1.0;
    let mut min_index: usize = 0;
    let mut index: usize = 0;
    for child in &node.children {
        sample = Beta::new(child.a, child.b).unwrap().sample(&mut rng);
        if sample < min_score {
            min_index = index;
            min_score = sample;
        }
        index += 1;
    }
    min_index
}

pub fn playout(node: &mut Node, result: &mut GameResult, passed: bool, table: &redb::ReadOnlyTable<(u64, u64), f32>){
    let legals: [u64; 8] = rule::get_movable(node.mine, node.oppo);
    if rule::get_legal(&legals) > 0 {
        if node.children.is_empty() {
            node.make_children(&legals, table);
        }
        let index_child = move_child(node);
        playout(&mut node.children[index_child], result, false, table);
        node.update_param(result);
    } else {
        if passed {
            end_game(node, result);
            node.update_param(result);
        } else {
            if node.children.is_empty() {
                node.children.push(Node::new(node.oppo, node.mine));
            }
            playout(&mut node.children[0], result, true, table);
            node.update_param(result);
        }
    }
}

fn reset_node(node: &mut Node) {
    let rate: f32 = (node.a + node.b) / (node.mine | node.oppo).count_ones() as f32;
    node.a = (node.a / rate) + 1.0;
    node.b = (node.b / rate) + 1.0;
    for child in &mut node.children {
        reset_node(child);
    }
}

fn flip_vertical(v: u64) -> u64 { v.swap_bytes() }
fn flip_horizontal(v: u64) -> u64 { v.reverse_bits().swap_bytes() }
fn rotate_180(v: u64) -> u64 { v.reverse_bits() }

fn canonicalize_stones(node: &Node) -> (u64, u64) {
    let directions: [(u64, u64); 4] = [
        (node.mine, node.oppo),
        (flip_vertical(node.mine), flip_vertical(node.oppo)),
        (flip_horizontal(node.mine), flip_horizontal(node.oppo)),
        (rotate_180(node.mine), rotate_180(node.oppo))
    ];
    // 本当はあと4方向あるけど…
    directions.into_iter().min().unwrap()
}

fn make_hashmap<'txn>(node: Node, table: &mut redb::Table<'txn, (u64, u64), f32>) {
    let hash = canonicalize_stones(&node);
    let p = node.a / (node.a + node.b);
    table.insert(hash, p).expect("キーバリューの書き込みに失敗しました。");
    for child in node.children {
        make_hashmap(child, table);
    }
    // ここでノードはメモリから消える？
}

fn read_hashmap(node: &mut Node, table: &redb::ReadOnlyTable<(u64, u64), f32>) {
    let key = canonicalize_stones(node);
    let pre_learn_rate: f32 = 2.0;
    let reading = table.get(key).expect("キーバリューの読み込みに失敗しました。");
    if let Some(p) = reading {
        let v = p.value();
        node.a += v * pre_learn_rate;
        node.b += (1.0 - v) * pre_learn_rate;
    };
}

fn count_node(node: &Node, count: &mut u32) {
    *count += 1;
    if !node.children.is_empty() {
        for child in &node.children {
            count_node(child, count);
        }
    }
}

fn print_node(node: &Node) {
    print!("\"mine\": {}, ", node.mine);
    print!("\"oppo\": {}, ", node.oppo);
    print!("\"alpha\": {:.3}, ", node.a);
    print!("\"beta\": {:.3}, ", node.b);
}

fn print_result(node: &Node) {
    print!("{{ ");
    print!("\"value\": {{ ");
    print_node(node);
    print!("\"children\": [ ");
    for child in &node.children {
        print!("{{ ");
        print_node(child);
        print!("\"move\": {}, ", (node.mine | node.oppo) ^ (child.mine | child.oppo));
        print!("}}, ");
    }
    print!("] ");
    print!("}} ");
    print!("}}");
}

fn prepare_database(path_db: &str) -> Database {
    let database = Database::create(path_db).expect("データベースが作れませんでした。"); // 既にあればopen
    let transaction = database.begin_write().expect("トランザクションを始められませんでした。");
    {
        transaction.open_table(NODES).expect("テーブルを開けませんでした。"); // 初期化に必要な処理
    }
    transaction.commit().expect("データベースへのコミットに失敗しました。");
    database
}

fn output_sample_nodes(table: ReadOnlyTable<(u64, u64), f32>) {

}

fn main() {
    let args: Vec<String> = env::args().collect();
    let mine_stones: u64 = args[1].parse().unwrap();
    let oppo_stones: u64 = args[2].parse().unwrap();
    let iter: u64 = args[3].parse().unwrap();
    let database = prepare_database("othello.redb");
    eprintln!("ノードデータベースを開きました。");
    let mut node: Node = Node::new(mine_stones, oppo_stones);
    let mut result: GameResult;
    let transaction = database.begin_read().expect("トランザクションを始められませんでした。");
    let table = transaction.open_table(NODES).expect("テーブルを開けませんでした。");
    eprintln!("{} ノードが記録されています。", table.len().expect("ノード数の取得に失敗しました。"));
    for _ in 0..iter {
        result = GameResult::None;
        playout(&mut node, &mut result, false, &table);
    }
    print_result(&node);
    eprintln!("探索が終了しました。");
    let transaction = database.begin_write().expect("トランザクションを始められませんでした。");
    {
        let mut table = transaction.open_table(NODES).expect("テーブルを開けませんでした。");
        make_hashmap(node, &mut table);
    }
    transaction.commit().expect("データベースへのコミットに失敗しました。");
    eprintln!("ノードデータベースの更新が終了しました。");
}
