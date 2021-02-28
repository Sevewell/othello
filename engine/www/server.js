const WebSocket = require('ws');
const fs = require('fs');
const { exec } = require('child_process');
const process = require('process');
const Computer = require('./search');

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

function putStone(ws, point) {

    // 着手可能箇所でなかったらターンは変えない

    if (table.turn == 'black') {

        const option = `${table.black} ${table.white} ${point}`;

        exec(`./move ${option}`, (err, stdout, stderr) => {

            const move = JSON.parse(stdout);
            table.black = to2From16(move.m);
            table.white = to2From16(move.y);
            table.turn = 'white';
    
        });
    
    }
    if (table.turn == 'white') {

        const option = `${table.white} ${table.black} ${point}`;

        exec(`./move ${option}`, (err, stdout, stderr) => {

            const move = JSON.parse(stdout);    
            table.white = to2From16(move.m);
            table.black = to2From16(move.y);
            table.turn = 'black';
    
        });
    
    }

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

server.listen(8080);
