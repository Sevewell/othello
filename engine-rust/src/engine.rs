mod rule;

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

    fn update_param(self: &mut Self, result: &mut GameResult, value: f32) -> f32 {
        match result {
            GameResult::Win => {
                self.a += value;
                *result = GameResult::Lose
            }
            GameResult::Lose => {
                self.b += value;
                *result = GameResult::Win;
            }
            GameResult::Draw => {
                self.a += value / 2.0;
                self.b += value / 2.0;
                *result = GameResult::Draw;
            }
            GameResult::None => panic!("不正なゲーム結果"),
        }
        value // 学習率で更新する予定
    }

    fn make_children(self: &mut Self, legals: &[u64; 8]) {
        let mut movable: u64 = rule::get_legal(legals);
        while movable > 0 {
            let lsb: u64 = movable & movable.wrapping_neg();
            let reversable: u64 = rule::get_reversable(self.oppo, lsb, legals);
            self.children.push(Node::new(self.oppo ^ reversable, self.mine | lsb | reversable));
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

fn end_game(node: &Node, result: &mut GameResult) -> f32 {
    let count_m: u32 = node.mine.count_ones();
    let count_y: u32 = node.oppo.count_ones();
    if count_m > count_y {
        *result = GameResult::Win;
    } else if count_m < count_y {
        *result = GameResult::Lose;
    } else {
        *result = GameResult::Draw;
    }
    1.0
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

pub fn playout(node: &mut Node, result: &mut GameResult, passed: bool) -> f32 {
    let mut value: f32;
    let legals: [u64; 8] = rule::get_movable(node.mine, node.oppo);
    if rule::get_legal(&legals) > 0 {
        if node.children.is_empty() {
            node.make_children(&legals);
        }
        let index_child = move_child(node);
        value = playout(&mut node.children[index_child], result, false);
        value = node.update_param(result, value);
    } else {
        if passed {
            value = end_game(node, result);
            value = node.update_param(result, value);
        } else {
            if node.children.is_empty() {
                node.children.push(Node::new(node.oppo, node.mine));
            }
            value = playout(&mut node.children[0], result, true);
            value = node.update_param(result, value);
        }
    }
    value
}