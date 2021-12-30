const { spawn } = require('child_process');
const readline = require('readline');
const { exec } = require('child_process');

module.exports = class Computer {

    num_process = parseInt(process.env.SEARCH_NODE);

    constructor() {
        this.power = false;
        this.process = 0;
        this.moves = [];
        this.playout = [];
        this.rate = [];
        this.node = [];
    }

    to2From16(str) {

        const move16bit = ('0'.repeat(16) + str).slice(-16);
        const move2bit = move16bit.split('').map((bits) => {
            const move2bits = parseInt(bits, 16).toString(2);
            return ('0000' + move2bits).slice(-4);
        });
        const bits = move2bit.join('');
        return bits;
    
    }
    
    summaryMoves(process) {

        const moves = process.reduce((moves, p) => {
    
            const move = moves.find((move) => {
                return move.move == p.move;
            });
        
            if (move) {
                move.count += 1;
            } else {
                moves.push({
                    move: p.move,
                    count: 1
                });
            }
    
            return moves;
            
        }, []);
    
        return moves;
    
    }
    
    choiceMove(moves) {
    
        const choice = moves.reduce((choice, move) => {
    
            if (move.count > choice.count) {
                return move;
            } else {
                return choice;
            }
    
        }, { move: '0', count: 0 });
    
        return choice;
    
    }

    move(table, choice) {

        const point = this.to2From16(choice.move).indexOf('1');
    
        this.moves = [];
        this.node = [];
        this.playout = [];
        this.rate = [];

        if (table.turn == 'black') {

            const option = `${table.black} ${table.white} ${point}`;

            exec(`./move ${option}`, (err, stdout, stderr) => {

                const move = JSON.parse(stdout);
                table.black = this.to2From16(move.m);
                table.white = this.to2From16(move.y);
                table.turn = 'white';
    
            });

        }
        if (table.turn == 'white') {

            const option = `${table.white} ${table.black} ${point}`;

            exec(`./move ${option}`, (err, stdout, stderr) => {

                const move = JSON.parse(stdout);
                table.white = this.to2From16(move.m);
                table.black = this.to2From16(move.y);
                table.turn = 'black';
    
            });

        }

    }
    
    streamSearch(table, record) {

        setTimeout(() => {
    
            const process = record.map((process) => {
                return process[process.length - 1];
            }).filter((p) => {
                return p;
                // undefinedになることがある
                // 出力がまだ一度も来ていない場合など
            });
    
            const moves = this.summaryMoves(process);
            const choice = this.choiceMove(moves);
    
            if (this.process == 0) {
    
                this.move(table, choice);
            
            } else {
    
                moves.forEach((move) => {
                    move.move = this.to2From16(move.move).indexOf('1')
                });
                this.moves = moves;
                
                this.playout = process.map((p) => {
                    return p.playout;
                });
                this.rate = process.map((p) => {
                    return p.rate;
                });
                this.node = process.map((p) => {
                    return p.node;
                });

                this.streamSearch(table, record);
    
            }
    
        }, 2000);
    
    }
    
    spawnSearch(table, record) {

        // プロセス毎の情報はまとめて送りたい
        const process_move = [];
        record.push(process_move);
    
        let m;
        let y;
        if (table.turn == 'black') {
            m = table.black;
            y = table.white;
        }
        if (table.turn == 'white') {
            m = table.white;
            y = table.black;
        }

        const seed = Math.floor(Math.random() * Math.floor(1000)).toString();
        const search = spawn('./search', [m, y, seed, this.learning_rate]);
    
        search.on('close', (code) => {
            this.process -= 1;
        });
    
        const rl = readline.createInterface(search.stdout);
    
        rl.on('line', (input) => {
    
            const data = JSON.parse(input);
            process_move.push(data);
        
        });
    
    }
    
    search(table) {

        if (this.process > 0) { return };
    
        const record = [];
    
        for (let i = 0; i < this.num_process; i++) {
            this.spawnSearch(table, record);
            this.process += 1;
        }
    
        // プロセスを作るのに時間がかかるっぽい
        // はじめのストリームまでの時間稼ぎ
        setTimeout(() => {
            this.streamSearch(table, record);
        }, 1000);

        this.power = true;
    
    }
    
};