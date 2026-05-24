use redb::{Database, ReadTransaction, ReadableDatabase, ReadableTableMetadata, TableDefinition, WriteTransaction};

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
        count_read: 0
    }
}

pub fn open_write_table<'txn>(transaction: &'txn WriteTransaction, mode: &str) -> Box<dyn WriteNodeStore + 'txn> {
    match mode {
        "write" => {
            Box::new(RedbWriteTable {
                table: transaction.open_table(NODES).expect("テーブルを開けませんでした。"),
                count_create: 0,
                count_update: 0
            })
        },
        "update" => {
            Box::new(RedbUpdateTable {
                table: transaction.open_table(NODES).expect("テーブルを開けませんでした。"),
                count_update: 0
            })
        },
        _ => {
            Box::new(RedbWriteTable {
                table: transaction.open_table(NODES).expect("テーブルを開けませんでした。"),
                count_create: 0,
                count_update: 0
            })
        }
    }
}

const NODES: TableDefinition<(u64, u64), (u16, u16)> = TableDefinition::new("nodes");

pub trait ReadNodeStore {
    fn read(&mut self, key: (u64, u64)) -> Option<(u16, u16)>;
}

pub struct RedbReadTable {
    table: redb::ReadOnlyTable<(u64, u64), (u16, u16)>,
    pub count_read: u64
}

impl ReadNodeStore for RedbReadTable {
    fn read(&mut self, key: (u64, u64)) -> Option<(u16, u16)> {
        match self.table.get(key).expect("キーバリューの読み込みに失敗しました。") {
            Some(kv) => {
                self.count_read += 1;
                Some(kv.value())
            },
            None => None
        }
    }
}

pub trait WriteNodeStore {
    fn write(&mut self, key: (u64, u64), value: (u16, u16));
    fn print(&self);
}

pub struct RedbWriteTable<'txn> {
    table: redb::Table<'txn, (u64, u64), (u16, u16)>,
    count_create: u64,
    count_update: u64
}

impl<'txn> WriteNodeStore for RedbWriteTable<'txn> {
    fn write(&mut self, key: (u64, u64), value: (u16, u16)) {
        match self.table.insert(key, value).expect("キーバリューの書き込みに失敗しました。") {
            Some(_kv) => {
                self.count_update += 1;
            },
            None => {
                self.count_create += 1;
            }
        }
    }
    fn print(&self) {
        eprintln!("{}のノードがDBに更新されます。", self.count_update);
        eprintln!("{}のノードがDBに登録されます。", self.count_create);
        // 取得の数と更新の数がなぜか合わない
    }
}

pub struct RedbUpdateTable<'txn> {
    table: redb::Table<'txn, (u64, u64), (u16, u16)>,
    count_update: u64
}

impl<'txn> WriteNodeStore for RedbUpdateTable<'txn> {
    fn write(&mut self, key: (u64, u64), value: (u16, u16)) {
        match self.table.get_mut(key).expect("キーバリューの読み込みに失敗しました。") {
            Some(mut kv) => {
                kv.insert(value).expect("キーバリューの書き込みに失敗しました。");
                self.count_update += 1;
            },
            None => {}
        }
    }
    fn print(&self) {
        eprintln!("{}のノードがDBに更新されます。", self.count_update);
    }
}
