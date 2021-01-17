const WebSocket = require('ws');
const fs = require('fs');
const { exec } = require('child_process');
const { spawn } = require('child_process');
const readline = require('readline');
const process = require('process');

if (process.env.CERT == 'true') {
    const https = require('https');
    const options = {
        key: fs.readFileSync('cert/key.pem'),
        cert: fs.readFileSync('cert/cert.pem')
    };
    var server = https.createServer(options);
} else {
    const http = require('http');
    var server = http.createServer();
}
const wss = new WebSocket.Server({ server });

const num_process = parseInt(process.env.SEARCH_NODE);

const status = {
    table: {
        seat: false,
        black: '0'.repeat(64),
        white: '0'.repeat(64),
        turn: null,
        time: 0,        
    },
    computing: {
        process: 0,
        search: [],
        playout: [],
        rate: [],
        node: []
    }
}

function takeSeat(ws) {

    if (status.table.seat) {

        if (ws.status.player) { // 離席
            ws.status.player = false;
            status.table.seat = false;
            status.table.turn = null;
            status.table.time = 0;
        }

    } else { // 着席

        ws.status.player = true;
        status.table.seat = true;
        status.table.black = '0'.repeat(28) + '1' + '0'.repeat(6) + '1' + '0'.repeat(28);
        status.table.white = '0'.repeat(27) + '1' + '0'.repeat(8) + '1' + '0'.repeat(27);
        status.table.turn = 'black';
        status.table.time = 0;
    }

}

function putStone(point) {

    if (status.computing.process > 0) {
        return;
    }

    const option = `${status.table.black} ${status.table.white} ${point}`;

    exec(`./move ${option}`, (err, stdout, stderr) => {

        const move = JSON.parse(stdout);
        status.table.black = to2From16(move.m);
        status.table.white = to2From16(move.y);

        search();

    });

}

function to2From16(str) {

    const move16bit = ('0'.repeat(16) + str).slice(-16);
    const move2bit = move16bit.split('').map((bits) => {
        const move2bits = parseInt(bits, 16).toString(2);
        return ('0000' + move2bits).slice(-4);
    });
    const bits = move2bit.join('');
    return bits;

}

function summaryMoves(process) {

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

function choiceMove(moves) {

    const choice = moves.reduce((choice, move) => {

        if (move.count > choice.count) {
            return move;
        } else {
            return choice;
        }

    }, { move: '0', count: 0 });

    return choice;

}

function streamSearch(record) {

    setTimeout(() => {

        const process = record.map((process) => {
            return process[process.length - 1];
        }).filter((p) => {
            return p;
            // undefinedになることがある
            // 出力がまだ一度も来ていない場合など
        });

        const moves = summaryMoves(process)
        const choice = choiceMove(moves);

        if (status.computing.process == 0) {

            const point = to2From16(choice.move).indexOf('1');

            status.computing.search = [];
            status.computing.node = [];
            status.computing.playout = [];
            status.computing.rate = [];

            const option = `${status.table.white} ${status.table.black} ${point}`;

            exec(`./move ${option}`, (err, stdout, stderr) => {

                const move = JSON.parse(stdout);
                status.table.white = to2From16(move.m);
                status.table.black = to2From16(move.y);

            });

            status.table.turn = 'black';
        
        } else {

            moves.forEach((move) => {
                move.move = to2From16(move.move).indexOf('1')
            });
            status.computing.search = moves;
            
            status.computing.playout = process.map((p) => {
                return p.playout;
            });
            status.computing.rate = process.map((p) => {
                return p.rate;
            });
            status.computing.node = process.map((p) => {
                return p.node;
            });
    
            streamSearch(record);

        }

    }, 2000);

}

function spawnSearch(record) {

    // プロセス毎の情報はまとめて送りたい
    const process_move = [];
    record.push(process_move);

    const seed = Math.floor(Math.random() * Math.floor(1000)).toString();
    const search = spawn('./search', [status.table.white, status.table.black, seed]);

    search.on('close', (code) => {
        status.computing.process -= 1;
    });

    const rl = readline.createInterface(search.stdout);

    rl.on('line', (input) => {

        const data = JSON.parse(input);
        process_move.push(data);
    
    });

}

function search() {

    if (status.computing.process != 0) {
        return;
    }

    status.table.turn = 'white';

    const record = [];

    for (let i = 0; i < num_process; i++) {
        spawnSearch(record);
        status.computing.process += 1;
    }

    // プロセスを作るのに時間がかかるっぽい
    // はじめのストリームまでの時間稼ぎ
    setTimeout(() => {
        streamSearch(record);
    }, 1000);

}

wss.on('connection', function connection(ws, req) {

    ws.status = {
        player: false
    };
    status.user = ws.status;
    ws.send(JSON.stringify(status));

    ws.on('message', function incoming(message) {

        message = JSON.parse(message);
        console.log(message);

        switch (message.key) {

            case 'open':
                break;
            case 'seat':
                takeSeat(ws);
                break;
            case 'move':
                if (ws.status.player) {
                    putStone(message.value);
                };
                break;

        }
    
    });

    ws.on('close', function close() {

        ;
        
    })
    
});

setInterval(() => {
    
    if (status.table.turn == 'black') {
        status.table.time -= 1;
    }
    if (status.table.turn == 'white') {
        status.table.time += 1;
    }

    wss.clients.forEach(function each(client) {
        status.user = client.status;
        client.send(JSON.stringify(status));
    });

}, 1000);

server.listen(8080);
