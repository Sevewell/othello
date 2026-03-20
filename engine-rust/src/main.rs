mod engine;

use std::env;

fn count_node(node: &engine::Node, count: &mut u32) {
    *count += 1;
    if !node.children.is_empty() {
        for child in &node.children {
            count_node(child, count);
        }
    }
}

fn print_node(node: engine::Node) {
    let mut nodes: u32 = 0;
    count_node(&node, &mut nodes);
    print!("[");
    for child in node.children {
        print!("{{ ");
        print!("\"move\": {}, ", (child.m | child.y) ^ (node.m | node.y));
        print!("\"m\": {}, ", child.y);
        print!("\"y\": {}, ", child.m);
        print!("\"alpha\": {:.3}", child.alpha);
        print!("\"nodes\": {}k", nodes / 1000);
        print!(" }}, ");
    }
    print!("]");
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let m: u64 = args[1].parse().unwrap();
    let y: u64 = args[2].parse().unwrap();
    let iter: u64 = args[3].parse().unwrap();
    let mut node: engine::Node = engine::Node::new(m, y);
    let mut result: engine::GameResult;
    for i in 0..iter {
        result = engine::GameResult::None;
        engine::playout(&mut node, &mut result, false);
    }
    print_node(node);
}
