use redb::{Database, Table, ReadOnlyTable, ReadTransaction, ReadableDatabase, TableDefinition, WriteTransaction};

const NODES: TableDefinition<(u64, u64), f32> = TableDefinition::new("nodes");

pub fn create_database(path_db: &str) -> Database {
    match Database::create(path_db) {
        Ok(db) => {
            eprintln!("データベースを作りました。");
            db
        }
        Err(error) => {
            panic!("データベースが作れませんでした。{:?}", error);
        }
    }
}

pub fn open_database(path_db: &str) -> Database {
    match Database::open(path_db) {
        Ok(db) => {
            eprintln!("データベースを読み込みました。");
            db
        }
        Err(error) => {
            panic!("データベースを読み込めませんでした。{:?}", error);
        }
    }
}

pub fn begin_read_transaction(db: &Database) -> ReadTransaction {
    match db.begin_read() {
        Ok(read_txn) => {
            read_txn
        }
        Err(error) => {
            panic!("トランザクションを始められませんでした。{:?}", error);
        }
    }
}

pub fn begin_write_transaction(db: &Database) -> WriteTransaction {
    match db.begin_write() {
        Ok(txn) => {
            txn
        }
        Err(error) => {
            panic!("トランザクションを始められませんでした。{:?}", error);
        }
    }
}

pub fn open_read_table(txn: &ReadTransaction) -> ReadOnlyTable<(u64, u64), f32> {
    match txn.open_table(NODES) {
        Ok(table) => {
            table
        }
        Err(error) => {
            panic!("テーブルを開けませんでした。{:?}", error);
        }
    }
}

pub fn open_write_table<'txn>(txn: &'txn WriteTransaction) -> Table<'txn, (u64, u64), f32> {
    match txn.open_table(NODES) {
        Ok(table) => {
            table
        }
        Err(error) => {
            panic!("テーブルを開けませんでした。{:?}", error);
        }
    }
}

pub fn read_kv(table: &ReadOnlyTable<(u64, u64), f32>, key: (u64, u64)) -> Option<f32> {
    match table.get(key) {
        Ok(reading) => {
            match reading {
                Some(guard) => Some(guard.value()),
                None => None,
            }
        }
        Err(error) => {
            panic!("キーバリューの読み込みに失敗しました。{:?}", error);
        }
    }
}
