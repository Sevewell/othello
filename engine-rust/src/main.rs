mod rule;

use std::env;
use std::fs;
use std::collections::HashMap;
use postcard;
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

    fn make_children(self: &mut Self, legals: &[u64; 8], hashmap: &mut HashMap<(u64, u64), f32>) {
        let mut movable: u64 = rule::get_legal(legals);
        let mut lsb: u64;
        let mut reversable: u64;
        let mut child: Node;
        while movable > 0 {
            lsb = movable & movable.wrapping_neg();
            reversable = rule::get_reversable(self.oppo, lsb, legals);
            child = Node::new(self.oppo ^ reversable, self.mine | lsb | reversable);
            pop_hashmap(&mut child, hashmap);
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

pub fn playout(node: &mut Node, result: &mut GameResult, passed: bool, hashmap: &mut HashMap<(u64, u64), f32>) {
    let legals: [u64; 8] = rule::get_movable(node.mine, node.oppo);
    if rule::get_legal(&legals) > 0 {
        if node.children.is_empty() {
            node.make_children(&legals, hashmap);
        }
        let index_child = move_child(node);
        playout(&mut node.children[index_child], result, false, hashmap);
        node.update_param(result);
    } else {
        if passed {
            end_game(node, result);
            node.update_param(result);
        } else {
            if node.children.is_empty() {
                node.children.push(Node::new(node.oppo, node.mine));
            }
            playout(&mut node.children[0], result, true, hashmap);
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

fn make_hashmap(node: Node, hashmap: &mut HashMap<(u64, u64), f32>) {
    let (mine_canonical, oppo_canonical) = canonicalize_stones(&node);
    hashmap.insert((mine_canonical, oppo_canonical),  node.a / (node.a + node.b));
    for child in node.children {
        make_hashmap(child, hashmap);
    }
    // ここでノードはメモリから消える？
}

fn pop_hashmap(node: &mut Node, hashmap: &mut HashMap<(u64, u64), f32>) {
    let (mine_canonical, oppo_canonical) = canonicalize_stones(node);
    let value: Option<f32> = hashmap.remove(&(mine_canonical, oppo_canonical));
    if value.is_some() {
        let p = value.unwrap();
        node.a += p;
        node.b += 1.0 - p;
    }
}

fn dump_hashmap(hashmap: HashMap<(u64, u64), f32>)  {
    let bytes: Vec<u8> = postcard::to_allocvec(&hashmap).unwrap();
    fs::write("tree.postcard", &bytes).unwrap();
    eprintln!("{} nodes are dumped.", hashmap.len());
}

fn load_hashmap() -> HashMap<(u64, u64), f32> {
    // どこかで切らないときついかも
    let reading = fs::read("tree.postcard");
    let bytes: Vec<u8> = match reading {
        Ok(file) => file,
        Err(error) => panic!("Problem opening the file: {:?}", error),
    };
    let hashmap: HashMap<(u64, u64), f32> = postcard::from_bytes(&bytes).unwrap();
    eprintln!("{} nodes are loaded.", hashmap.len());
    hashmap
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
    let mut nodes: u32 = 0;
    count_node(&node, &mut nodes);
    print!("{{ ");
    print!("\"value\": {{ ");
    print_node(node);
    print!("\"children\": [ ");
    for child in &node.children {
        print!("{{ ");
        print_node(child);
        print!("}}, ");
    }
    print!("] ");
    print!("}} ");
    print!("}}");
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let mine_stones: u64 = args[1].parse().unwrap();
    let oppo_stones: u64 = args[2].parse().unwrap();
    let iter: u64 = args[3].parse().unwrap();
    let mut hashmap: HashMap<(u64, u64), f32> = load_hashmap();
    //let mut hashmap: HashMap<(u64, u64), f32> = HashMap::new();
    let mut node: Node = Node::new(mine_stones, oppo_stones);
    let mut result: GameResult;
    for _ in 0..iter {
        result = GameResult::None;
        playout(&mut node, &mut result, false, &mut hashmap);
    }
    print_result(&node);
    make_hashmap(node, &mut hashmap);
    // 局面の方向による正規化でデータ量を圧縮できる
    // 180°はビット反転
    // 上下はバイト列にして反転
    // 左右はバイト列毎にビット反転
    // でも圧縮するとpopのトリガーが面倒だなあ
    // やるなら母数はハッシュマップで持つくらいしないと
    dump_hashmap(hashmap);
}
