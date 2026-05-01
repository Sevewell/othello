use redb::{Database, ReadTransaction, ReadableDatabase, TableDefinition, WriteTransaction, ReadableTableMetadata};

pub fn prepare_database(path_db: &str) -> Database {
    let database = Database::create(path_db).expect("データベースを展開できませんでした。"); // 既にあればopen
    eprintln!("ノードデータベースを開きました。");
    let transaction = database.begin_write().expect("トランザクションを始められませんでした。");
    {
        let table = transaction.open_table(NODES).expect("テーブルを開けませんでした。"); // 初期化に必要な処理
        eprintln!("{}のノードが記録されています。", table.len().expect("ノード数の取得に失敗しました。"));
    }
    transaction.commit().expect("データベースへのコミットに失敗しました。");
    database
}

pub fn create_read_transaction(database: &Database) -> redb::ReadTransaction {
    database.begin_read().expect("トランザクションを始められませんでした。")
}

pub fn create_write_transaction(database: &Database) -> redb::WriteTransaction {
    database.begin_write().expect("トランザクションを始められませんでした。")
}

pub fn open_read_table(transaction: &ReadTransaction) -> RedbReadTable {
    RedbReadTable {
        table: transaction.open_table(NODES).expect("テーブルを開けませんでした。"),
    }
}

pub fn open_write_table(transaction: &WriteTransaction) -> RedbWriteTable<'_> {
    RedbWriteTable {
        table: transaction.open_table(NODES).expect("テーブルを開けませんでした。"),
    }
}

const NODES: TableDefinition<(u64, u64), f32> = TableDefinition::new("nodes");

pub trait ReadNodeStore {
    fn read(&self, key: (u64, u64)) -> Option<f32>;
}

pub trait WriteNodeStore {
    fn write(&mut self, key: (u64, u64), value: f32) -> Option<f32>;
}

pub struct RedbReadTable {
    table: redb::ReadOnlyTable<(u64, u64), f32>,
}

impl ReadNodeStore for RedbReadTable {
    fn read(&self, key: (u64, u64)) -> Option<f32> {
        self.table
            .get(key)
            .expect("キーバリューの読み込みに失敗しました。")
            .map(|p| p.value())
    }
}

pub struct RedbWriteTable<'txn> {
    table: redb::Table<'txn, (u64, u64), f32>,
}

impl<'txn> WriteNodeStore for RedbWriteTable<'txn> {
    fn write(&mut self, key: (u64, u64), value: f32) -> Option<f32> {
        self.table
            .insert(key, value)
            .expect("キーバリューの書き込みに失敗しました。")
            .map(|p| p.value())
    }
}