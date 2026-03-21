/// オセロのルール計算モジュール
/// ビットボード表現を使用した合法手・反転石の計算

/// 左シフト方向の合法手計算（上・左・左上・左下斜め）
fn get_movable_l(mine: u64, oppo: u64, shift: u32, mask: u64) -> u64 {
    let mut legal = (mine << shift) & mask;
    legal |= (legal << shift) & mask;
    legal |= (legal << shift) & mask;
    legal |= (legal << shift) & mask;
    legal |= (legal << shift) & mask;
    legal |= (legal << shift) & mask;
    (legal << shift) & !(mine | oppo)
}

/// 右シフト方向の合法手計算（下・右・右上・右下斜め）
fn get_movable_r(mine: u64, oppo: u64, shift: u32, mask: u64) -> u64 {
    let mut legal = (mine >> shift) & mask;
    legal |= (legal >> shift) & mask;
    legal |= (legal >> shift) & mask;
    legal |= (legal >> shift) & mask;
    legal |= (legal >> shift) & mask;
    legal |= (legal >> shift) & mask;
    (legal >> shift) & !(mine | oppo)
}

/// 8方向の合法手をlegals配列に格納
/// legals[0..=3]: 左シフト方向（水平・斜め・垂直・斜め）
/// legals[4..=7]: 右シフト方向（水平・斜め・垂直・斜め）
pub fn get_movable(mine: u64, oppo: u64) -> [u64; 8] {
    // こちらで配列を作っては？
    let mut legals: [u64; 8] = [0; 8];
    let mask_horizontal: u64 = oppo & 9_114_861_777_597_660_798;
    let mask_vertical: u64   = oppo & 72_057_594_037_927_680;
    let mask_diagonal: u64   = oppo & 35_604_928_818_740_736;
    legals[0] = get_movable_l(mine, oppo, 1, mask_horizontal);
    legals[1] = get_movable_l(mine, oppo, 9, mask_diagonal);
    legals[2] = get_movable_l(mine, oppo, 8, mask_vertical);
    legals[3] = get_movable_l(mine, oppo, 7, mask_diagonal);
    legals[4] = get_movable_r(mine, oppo, 1, mask_horizontal);
    legals[5] = get_movable_r(mine, oppo, 9, mask_diagonal);
    legals[6] = get_movable_r(mine, oppo, 8, mask_vertical);
    legals[7] = get_movable_r(mine, oppo, 7, mask_diagonal);
    legals
}

/// 8方向の合法手ビットボードをORで合算して返す
pub fn get_legal(legals: &[u64; 8]) -> u64 {
    legals[0] | legals[1] | legals[2] | legals[3] | legals[4] | legals[5] | legals[6] | legals[7]
}

/// 左シフト方向の反転石計算
fn get_reversable_l(oppo: u64, mv: u64, shift: u32, legal: u64) -> u64 {
    let mut flipped = ((mv & legal) >> shift) & oppo;
    flipped |= (flipped >> shift) & oppo;
    flipped |= (flipped >> shift) & oppo;
    flipped |= (flipped >> shift) & oppo;
    flipped |= (flipped >> shift) & oppo;
    flipped |= (flipped >> shift) & oppo;
    flipped
}

/// 右シフト方向の反転石計算
fn get_reversable_r(oppo: u64, mv: u64, shift: u32, legal: u64) -> u64 {
    let mut flipped = ((mv & legal) << shift) & oppo;
    flipped |= (flipped << shift) & oppo;
    flipped |= (flipped << shift) & oppo;
    flipped |= (flipped << shift) & oppo;
    flipped |= (flipped << shift) & oppo;
    flipped |= (flipped << shift) & oppo;
    flipped
}

/// 8方向すべての反転石をビットボードで返す
pub fn get_reversable(oppo: u64, mv: u64, legals: &[u64; 8]) -> u64 {
    let mut flipped: u64 = 0;
    flipped |= get_reversable_l(oppo, mv, 1, legals[0]);
    flipped |= get_reversable_l(oppo, mv, 9, legals[1]);
    flipped |= get_reversable_l(oppo, mv, 8, legals[2]);
    flipped |= get_reversable_l(oppo, mv, 7, legals[3]);
    flipped |= get_reversable_r(oppo, mv, 1, legals[4]);
    flipped |= get_reversable_r(oppo, mv, 9, legals[5]);
    flipped |= get_reversable_r(oppo, mv, 8, legals[6]);
    flipped |= get_reversable_r(oppo, mv, 7, legals[7]);
    flipped
}
