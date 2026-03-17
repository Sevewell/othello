pub struct Node {
    m: u64,
    y: u64,
    alpha: f32,
    next: Option<Box<Node>>,
    child: Option<Box<Node>>
}

impl Node {
    pub fn new(m: u64, y: u64) -> Self {
        Node {
            m: m,
            y: y,
            alpha: 1.,
            next: None,
            child: None,
        }
    }
    fn update(&mut self, result: &mut GameResult, value: f32) -> f32 {
        match result {
            GameResult::Win => {
                self.alpha += value;
                *result = GameResult::Lose;
            }
            GameResult::Lose => {
                *result = GameResult::Win;
            }
            GameResult::Draw => {
                self.alpha += value / 2.0;
                *result = GameResult::Draw;
            }
            GameResult::None => panic!("不正なゲーム結果"),
        }
        value
    }
}

pub enum GameResult {
    Win,
    Lose,
    Draw,
    None,
}