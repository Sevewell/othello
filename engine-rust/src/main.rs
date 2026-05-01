mod rule;
mod store;

use std::env;
use rand::rngs::SmallRng;
use rand_distr::{Distribution, Beta};

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

    fn make_children(self: &mut Self, legals: &[u64; 8], store: &impl store::ReadNodeStore) {
        let mut movable: u64 = rule::get_legal(legals);
        let mut lsb: u64;
        let mut reversable: u64;
        let mut child: Node;
        while movable > 0 {
            lsb = movable & movable.wrapping_neg();
            reversable = rule::get_reversable(self.oppo, lsb, legals);
            child = Node::new(self.oppo ^ reversable, self.mine | lsb | reversable);
            read_hashmap(&mut child, store);
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
    for child in &node.children { // ここで読みきりを検知したい
        sample = Beta::new(child.a, child.b).unwrap().sample(&mut rng);
        if sample < min_score {
            min_index = index;
            min_score = sample;
        }
        index += 1;
    }
    // 読みきりが起きればテーブルに登録する
    min_index
}

pub fn playout(node: &mut Node, result: &mut GameResult, passed: bool, store: &impl store::ReadNodeStore){
    let legals: [u64; 8] = rule::get_movable(node.mine, node.oppo);
    // 読みきりテーブルを見る
    if rule::get_legal(&legals) > 0 {
        if node.children.is_empty() {
            node.make_children(&legals, store);
        }
        let index_child = move_child(node);
        playout(&mut node.children[index_child], result, false, store);
        node.update_param(result);
    } else {
        if passed {
            end_game(node, result);
            // 読みきりテーブルに登録する
            node.update_param(result);
        } else {
            if node.children.is_empty() {
                node.children.push(Node::new(node.oppo, node.mine));
            }
            playout(&mut node.children[0], result, true, store);
            node.update_param(result);
        }
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

fn make_hashmap<'txn>(node: Node, store: &mut impl store::WriteNodeStore, min_visits: f32) -> (u64, u64) {
    let visits = node.a + node.b - 2.0;
    if visits < min_visits {
        return (0, 0)
    }
    let hash = canonicalize_stones(&node);
    let p = node.a / (node.a + node.b);
    let mut count_node_update: u64 = 0;
    let mut count_node_create: u64 = 0;
    {
        // 期待値だけに情報を落とすのは学習が弱いかも
        let writed = store.write(hash, p);
        match writed {
            Some(_p) => count_node_update += 1,
            None => count_node_create += 1
        }
        // 書き込み結果を次の前にdropしなきゃいけないみたい
    }
    let mut count_child_update: u64;
    let mut count_child_create: u64;
    for child in node.children {
        (count_child_update, count_child_create) = make_hashmap(child, store, min_visits);
        count_node_update += count_child_update;
        count_node_create += count_child_create;
    }
    (count_node_update, count_node_create)
    // ここでノードはメモリから消える？
}

fn read_hashmap(node: &mut Node, store: &impl store::ReadNodeStore) {
    let key = canonicalize_stones(node);
    let pre_learn_rate: f32 = 2.0;
    // 学習レートをかけるだけでは学習が弱いかも
    let reading = store.read(key);
    if let Some(p) = reading {
        node.a += p * pre_learn_rate;
        node.b += (1.0 - p) * pre_learn_rate;
    };
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
    println!("}}");
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let mine_stones: u64 = args[1].parse().unwrap();
    let oppo_stones: u64 = args[2].parse().unwrap();
    let iter: u64 = args[3].parse().unwrap();
    let database = store::prepare_database("othello.redb");
    let mut node: Node = Node::new(mine_stones, oppo_stones);
    let mut result: GameResult;
    let transaction = store::create_read_transaction(&database);
    let table = store::open_read_table(&transaction);
    eprintln!("探索を開始します。");
    for _ in 0..iter {
        result = GameResult::None;
        playout(&mut node, &mut result, false, &table);
    }
    eprintln!("探索が終了しました。");
    print_result(&node);
    // 書き込みをスキップするオプションがあってもいいかも
    let transaction = store::create_write_transaction(&database);
    {
        let mut table = store::open_write_table(&transaction);
        let (count_node_update, count_node_create) = make_hashmap(node, &mut table, 2.0);
        eprintln!("{}のノードがDBに更新されました。", count_node_update);
        eprintln!("{}のノードがDBに登録されました。", count_node_create);
    }
    transaction.commit().expect("データベースへのコミットに失敗しました。");
    eprintln!("ノードデータベースの更新が終了しました。");
}
