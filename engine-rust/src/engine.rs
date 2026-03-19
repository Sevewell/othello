mod rule;

use std::f64::INFINITY;
use rand_distr::{Distribution, Gamma};

pub struct Node {
    m: u64,
    y: u64,
    alpha: f32,
    children: Vec<Node>
}

impl Node {
    pub fn new(m: u64, y: u64) -> Self {
        Node {
            m: m,
            y: y,
            alpha: 1.0,
            children: Vec::new(),
        }
    }
    fn update_param(self: &mut Self, result: &mut GameResult, value: f32) -> f32 {
        match result {
            GameResult::Win => {
                self.alpha += value;
                *result = GameResult::Lose;
            }
            GameResult::Lose => {
                self.alpha -= value;
                self.alpha = self.alpha.max(1.0);
                *result = GameResult::Win;
            }
            GameResult::Draw => {
                self.alpha += value / 2.0;
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
            let reversable: u64 = rule::get_reversable(self.m, self.y, lsb, legals);
            self.children.push(Node::new(self.y ^ reversable, self.m | lsb | reversable));
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
    let count_m: u32 = node.m.count_ones();
    let count_y: u32 = node.y.count_ones();
    if count_m > count_y {
        *result = GameResult::Win;
    } else if count_m < count_y {
        *result = GameResult::Lose;
    } else {
        *result = GameResult::Draw;
    }
    1.0
}

fn move_child(node: &Node, legals: &[u64; 8]) -> usize {
    let mut movable: u64 = rule::get_legal(legals);
    let mut gamma: Gamma<f64>;
    let mut sample: f64;
    let mut min_score: f64 = INFINITY;
    let mut min_index: usize = 0;
    let mut index: usize = 0;
    for child in &node.children {
        gamma = Gamma::new(child.alpha as f64, 1.0).unwrap();
        sample = gamma.sample(&mut rand::rng());
        if sample <= min_score {
            min_index = index;
            min_score = sample;
        }
        movable ^= movable & (child.m | child.y);
        index += 1;
    }
    min_index
}

pub fn playout(node: &mut Node, result: &mut GameResult, passed: bool) -> f32 {
    let mut value: f32;
    let legals: [u64; 8] = rule::get_movable(node.m, node.y);
    if rule::get_legal(&legals) > 0 {
        if node.children.is_empty() {
            node.make_children(&legals);
        }
        let index_child = move_child(node, &legals);
        value = playout(&mut node.children[index_child], result, false);
        value = node.update_param(result, value);
    } else {
        if passed {
            value = end_game(node, result);
            value = node.update_param(result, value);
        } else {
            if node.children.is_empty() {
                node.children.push(Node::new(node.y, node.m));
            }
            value = playout(&mut node.children[0], result, true);
        }
    }
    value
}