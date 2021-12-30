const WebSocket = require('ws');
const fs = require('fs');
const { exec } = require('child_process');
const process = require('process');
const Computer = require('./search');

let server = undefined;

if (process.env.CERT == 'true') {
    const https = require('https');
    const options = {
        key: fs.readFileSync('cert/key.pem'),
        cert: fs.readFileSync('cert/cert.pem')
    };
    server = https.createServer(options);
} else {
    const http = require('http');
    server = http.createServer((req, res) => {

        console.log(req.url);

        let data = ''

        req.on('data', (chunk) => {
            data += chunk;
        });

        req.on('end', () => {

            console.log(data);
            data = JSON.parse(data);

            switch (req.url) {

                case '/move':
                    putStone(res, data);
                    break;

                case '/search':
                    startSearch(res, data);
                    break;

                case '/progress':
                    viewProgress(res, data);
                    break;

            }

        });

    });
}
const wss = new WebSocket.Server({ server });

const table = {

    turn: 'black',
    black: '0'.repeat(64),
    white: '0'.repeat(64)

};

const player = {

    black: {
        ws: null,
        time: null,
        move: null,
        com: new Computer()
    },
    white: {
        ws: null,
        time: null,
        move: null,
        com: new Computer()
    }

};

const com = new Computer();

function takeSeat(ws, turn) {

    if (player[turn].ws) {

        if (ws === player[turn].ws) {
            player[turn].ws = null;
            ws.status[turn] = false;
        }

    } else {

        player[turn].ws = ws;

        table.turn = 'black';
        player.black.time = 0;
        player.white.time = 0;

        ws.status[turn] = true;

        if (turn = 'black') {
            table.black = '0'.repeat(28) + '1' + '0'.repeat(6) + '1' + '0'.repeat(28);
        }
        if (turn = 'white') {
            table.white = '0'.repeat(27) + '1' + '0'.repeat(8) + '1' + '0'.repeat(27);
        }

    }

}

function putStone(res, data) {

    // 着手可能箇所でなかったらターンは変えたくない

    if (data.table.turn == 'black') {

        const option = `${data.table.black} ${data.table.white} ${data.move}`;

        exec(`./move ${option}`, (err, stdout, stderr) => {

            const move = JSON.parse(stdout);
            data.table.black = to2From16(move.m);
            data.table.white = to2From16(move.y);
            data.table.turn = 'white';

            res.writeHead(200, { 'Content-Type': 'application/json'} );
            res.write(JSON.stringify(data.table));
            res.end();
    
        });
    
    }
    if (data.table.turn == 'white') {

        const option = `${data.table.white} ${data.table.black} ${data.move}`;

        exec(`./move ${option}`, (err, stdout, stderr) => {

            const move = JSON.parse(stdout);    
            data.table.white = to2From16(move.m);
            data.table.black = to2From16(move.y);
            data.table.turn = 'black';

            res.writeHead(200, { 'Content-Type': 'application/json'} );
            res.write(JSON.stringify(data.table));
            res.end();

        });
    
    }

}

function startSearch(res, req_body) {

    const res_body = {
        status: undefined
    };

    if (!com.process) {
        com.search(req_body.table);
        res_body.status = 'kicked';
    } else {
        res_body.status = 'no kicked because resouce is used.'
    }

    res.writeHead(200, { 'Content-Type': 'application/json'} );

    res.write(JSON.stringify(res_body));
    res.end();

}

function viewProgress(res, req_body) {

    res.writeHead(200, { 'Content-Type': 'application/json'} );

    const res_body = {
        process: com.process,
        moves: com.moves,
        playout: com.playout,
        rate: com.rate,
        node: com.node
    };

    res.write(JSON.stringify(res_body));
    res.end();

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

wss.on('connection', function connection(ws, req) {

    ws.status = {
        black: false,
        white: false
    };

    ws.on('message', function incoming(message) {

        message = JSON.parse(message);
        console.log(message);

        switch (message.key) {

            case 'open':
                break;
            case 'seat':
                takeSeat(ws, message.value);
                break;
            case 'switch':
                if (ws === player[table.turn].ws) {
                    player[table.turn].com.learning_rate = message.value;
                    player[table.turn].com.search(table);
                };
                break;
            case 'move':
                if (ws === player[table.turn].ws) {
                    putStone(ws, message.value);
                }
                break;

        }
    
    });

    ws.on('close', function close() {

        ;
        
    })
    
});

setInterval(() => {

    player[table.turn].time += 1;

    const status = {

        turn: table.turn,
        black: {
            stone: table.black,
            time: player['black'].time,
            player: Boolean(player['black'].ws)
        },
        white: {
            stone: table.white,
            time: player['white'].time,
            player: Boolean(player['white'].ws)
        }

    }

    status.black.com = {
        search: player['black'].com.moves,
        playout: player['black'].com.playout,
        rate: player['black'].com.rate,
        node: player['black'].com.node
    };
    status.white.com = {
        search: player['white'].com.moves,
        playout: player['white'].com.playout,
        rate: player['white'].com.rate,
        node: player['white'].com.node
    };

    wss.clients.forEach(function each(client) {
        status.user = client.status;
        client.send(JSON.stringify(status));
    });

}, 1000);

server.listen(parseInt(process.env.PORT));
