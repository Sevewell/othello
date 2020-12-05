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
    seat: false,
    black: '0'.repeat(28) + '1' + '0'.repeat(6) + '1' + '0'.repeat(28),
    white: '0'.repeat(27) + '1' + '0'.repeat(8) + '1' + '0'.repeat(27),
    rate: [],
    computing: 0
}

const ping = [];

function takeSeat(ws) {

    if (status.seat) {

        if (ws.status.player) { // 離席
            ws.status.player = false;
            status.seat = false;
        }

    } else { // 着席

        ws.status.player = true;
        status.seat = true;

    }

    wss.clients.forEach(function each(client) {
        client.send(JSON.stringify({
            field: status,
            user: client.status
        }));
    });

}

function putStone(point) {

    const option = `${status.black} ${status.white} ${point}`;

    exec(`./move ${option}`, (err, stdout, stderr) => {

        const move = JSON.parse(stdout);
        status.black = to2From16(move.m);
        status.white = to2From16(move.y);

        wss.clients.forEach(function each(client) {
            client.send(JSON.stringify({
                field: status,
                user: client.status
            }));
        });

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

function countMove(move, process_last) {

    const count = process_last.filter((p) => {
        return p.move == move.move;
    }).length;

    return count;

}

function choiceMove(record) {

    const process_last = record.map((process) => {
        return process.pop();
    });
    console.log(process_last);

    const move = process_last.reduce((choice, option) => {
        const count_option = countMove(option, process_last);
        const count_choice = countMove(choice, process_last);
        if (count_option > count_choice) {
            return option;
        } else {
            return choice;
        }
    });

    return move;

}

function streamSearch(record) {

    setTimeout(() => {

        const choice = choiceMove(record);
        console.log(choice);

        if (status.computing == 0) {

            const point = to2From16(choice.move).indexOf('1');
            status.rate = [];

            const option = `${status.white} ${status.black} ${point}`;

            exec(`./move ${option}`, (err, stdout, stderr) => {

                const move = JSON.parse(stdout);
                status.white = to2From16(move.m);
                status.black = to2From16(move.y);

                wss.clients.forEach(function each(client) {
                    client.send(JSON.stringify({
                        field: status,
                        user: client.status
                    }));
                });    

            });
        
        } else {

            status.rate = record.map((process) => {
                return process.pop().rate;
            });

            wss.clients.forEach(function each(client) {
                client.send(JSON.stringify({
                    field: status,
                    user: client.status
                }));
            });

            streamSearch(record);

        }

    }, 1000);

}

function spawnSearch(record) {

    // プロセス毎の情報はまとめて送りたい
    const process_move = [];
    record.push(process_move);

    const seed = Math.floor(Math.random() * Math.floor(1000)).toString();
    const search = spawn('./search', [status.white, status.black, seed]);

    search.on('close', (code) => {
        console.log(`close: ${code}`);
        const end_time = new Date();
        //console.log((end_time - start_time) / 1000);
        status.computing -= 1;
    });

    const rl = readline.createInterface(search.stdout);

    rl.on('line', (input) => {

        const data = JSON.parse(input);
        process_move.push(data);
    
    });

}

function search() {

    if (status.computing != 0) {
        return;
    }

    const record = [];

    for (let i = 0; i < num_process; i++) {
        spawnSearch(record);
        status.computing += 1;
    }

    streamSearch(record);

}

wss.on('connection', function connection(ws, req) {

    ws.status = {
        player: false
    };
    ws.send(JSON.stringify({
        field: status,
        user: ws.status
    }));

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
            case 'search':
                if (ws.status.player) {
                    search();
                };
                return;

        }
    
    });

    ws.on('close', function close() {

        ;
        
    })
    
});

server.listen(8080);
