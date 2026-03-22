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

fn print_node(node: &engine::Node) {
    print!("\"mine\": {}, ", node.mine);
    print!("\"oppo\": {}, ", node.oppo);
    print!("\"alpha\": {:.3}, ", node.a);
    print!("\"beta\": {:.3}, ", node.b);
}

fn print_result(node: &engine::Node) {
    let mut nodes: u32 = 0;
    count_node(&node, &mut nodes);
    print!("{{ ");
    print!("\"nodes\": \"{}万\", ", nodes / 10000);
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

fn reset_node(node: &mut engine::Node) {
    let rate: f32 = (node.a + node.b) / (node.mine | node.oppo).count_ones() as f32;
    node.a = (node.a / rate) + 1.0;
    node.b = (node.b / rate) + 1.0;
    for child in &mut node.children {
        reset_node(child);
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let mine_stones: u64 = args[1].parse().unwrap();
    let oppo_stones: u64 = args[2].parse().unwrap();
    let epic: u64 = args[3].parse().unwrap();
    let iter: u64 = args[4].parse().unwrap();
    let mut node: engine::Node = engine::Node::new(mine_stones, oppo_stones);
    let mut result: engine::GameResult;
    for i in 0..epic {
        for _ in 0..iter {
            result = engine::GameResult::None;
            engine::playout(&mut node, &mut result, false);
        }
        if i < (epic - 1) {
            reset_node(&mut node);
        }
    }
    print_result(&node);
}
