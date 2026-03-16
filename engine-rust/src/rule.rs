/// オセロのルール計算モジュール
/// ビットボード表現を使用した合法手・反転石の計算

/// 左シフト方向の合法手計算（上・左・左上・左下斜め）
fn get_movable_l(m: u64, y: u64, shift: u32, mask: u64) -> u64 {
    let mut legal = (m << shift) & mask;
    legal |= (legal << shift) & mask;
    legal |= (legal << shift) & mask;
    legal |= (legal << shift) & mask;
    legal |= (legal << shift) & mask;
    legal |= (legal << shift) & mask;
    (legal << shift) & !(m | y)
}

/// 右シフト方向の合法手計算（下・右・右上・右下斜め）
fn get_movable_r(m: u64, y: u64, shift: u32, mask: u64) -> u64 {
    let mut legal = (m >> shift) & mask;
    legal |= (legal >> shift) & mask;
    legal |= (legal >> shift) & mask;
    legal |= (legal >> shift) & mask;
    legal |= (legal >> shift) & mask;
    legal |= (legal >> shift) & mask;
    (legal >> shift) & !(m | y)
}

/// 8方向の合法手をlegals配列に格納
/// legals[0..=3]: 左シフト方向（水平・斜め・垂直・斜め）
/// legals[4..=7]: 右シフト方向（水平・斜め・垂直・斜め）
pub fn get_movable(m: u64, y: u64, legals: &mut [u64; 8]) {
    let mask_horizontal: u64 = y & 9_114_861_777_597_660_798;
    let mask_vertical: u64   = y & 72_057_594_037_927_680;
    let mask_diagonal: u64   = y & 35_604_928_818_740_736;
    legals[0] = get_movable_l(m, y, 1, mask_horizontal);
    legals[1] = get_movable_l(m, y, 9, mask_diagonal);
    legals[2] = get_movable_l(m, y, 8, mask_vertical);
    legals[3] = get_movable_l(m, y, 7, mask_diagonal);
    legals[4] = get_movable_r(m, y, 1, mask_horizontal);
    legals[5] = get_movable_r(m, y, 9, mask_diagonal);
    legals[6] = get_movable_r(m, y, 8, mask_vertical);
    legals[7] = get_movable_r(m, y, 7, mask_diagonal);
}

/// 8方向の合法手ビットボードをORで合算して返す
pub fn get_legal(legals: &[u64; 8]) -> u64 {
    legals[0] | legals[1] | legals[2] | legals[3]
        | legals[4] | legals[5] | legals[6] | legals[7]
}

/// 左シフト方向の反転石計算
fn get_reversable_l(y: u64, mv: u64, shift: u32, legal: u64) -> u64 {
    let mut flipped = ((mv & legal) >> shift) & y;
    flipped |= (flipped >> shift) & y;
    flipped |= (flipped >> shift) & y;
    flipped |= (flipped >> shift) & y;
    flipped |= (flipped >> shift) & y;
    flipped |= (flipped >> shift) & y;
    flipped
}

/// 右シフト方向の反転石計算
fn get_reversable_r(y: u64, mv: u64, shift: u32, legal: u64) -> u64 {
    let mut flipped = ((mv & legal) << shift) & y;
    flipped |= (flipped << shift) & y;
    flipped |= (flipped << shift) & y;
    flipped |= (flipped << shift) & y;
    flipped |= (flipped << shift) & y;
    flipped |= (flipped << shift) & y;
    flipped
}

/// 8方向すべての反転石をビットボードで返す
pub fn get_reversable(m: u64, y: u64, mv: u64, legals: &[u64; 8]) -> u64 {
    let _ = m; // 現在は未使用（将来の拡張用）
    let mut flipped: u64 = 0;
    flipped |= get_reversable_l(y, mv, 1, legals[0]);
    flipped |= get_reversable_l(y, mv, 9, legals[1]);
    flipped |= get_reversable_l(y, mv, 8, legals[2]);
    flipped |= get_reversable_l(y, mv, 7, legals[3]);
    flipped |= get_reversable_r(y, mv, 1, legals[4]);
    flipped |= get_reversable_r(y, mv, 9, legals[5]);
    flipped |= get_reversable_r(y, mv, 8, legals[6]);
    flipped |= get_reversable_r(y, mv, 7, legals[7]);
    flipped
}
