const process = require('process');
const fs = require('fs');
const path = require('path');
const WebSocket = require('ws');
const { exec } = require('child_process');
const Computer = require('./search');

let server = undefined;
let port = undefined;

if (process.env.CERT == 'true') {
    const https = require('https');
    const options = {
        key: fs.readFileSync('cert/key.pem'),
        cert: fs.readFileSync('cert/cert.pem')
    };
    server = https.createServer(options, request_);
    port = 443;
} else {
    server = require('http').createServer(request_);
    port = 80;
}

const wss = new WebSocket.Server({ server });
server.listen(port);

function request_(request, response) {

    console.log('request ', request.url);

    var filePath = './www' + request.url;
    if (filePath == './www/') {
        filePath = './www/index.html';
    }

    var extname = String(path.extname(filePath)).toLowerCase();
    var mimeTypes = {
        '.html': 'text/html',
        '.js': 'text/javascript',
        '.css': 'text/css',
        '.json': 'application/json',
        '.png': 'image/png',
        '.jpg': 'image/jpg',
        '.gif': 'image/gif',
        '.wav': 'audio/wav',
        '.mp4': 'video/mp4',
        '.woff': 'application/font-woff',
        '.ttf': 'application/font-ttf',
        '.eot': 'application/vnd.ms-fontobject',
        '.otf': 'application/font-otf',
        '.svg': 'application/image/svg+xml'
    };

    var contentType = mimeTypes[extname] || 'application/octet-stream';

    fs.readFile(filePath, function(error, content) {
        if (error) {
            if(error.code == 'ENOENT') {
                fs.readFile('./404.html', function(error, content) {
                    response.writeHead(200, { 'Content-Type': contentType });
                    response.end(content, 'utf-8');
                });
            }
            else {
                response.writeHead(500);
                response.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
                response.end();
            }
        }
        else {
            response.writeHead(200, { 'Content-Type': contentType });
            response.end(content, 'utf-8');
        }
    });

};

const http = require('http');

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

    const data = JSON.stringify({
        table: table,
        move: point
    });

    const options = {
        hostname: 'engine',
        port: 8080,
        path: '/move',
        method: 'GET',
        headers: {
            'Content-Type': 'application/json',
            'Content-Length': data.length
        }
    };

    const req = http.request(options, (res) => {

        console.log(`StatusCode: ${res.statusCode}`);

        res.on('data', (d) => {

            table_updated = JSON.parse(d);
            console.log(table_updated);
            table.black = table_updated.black;
            table.white = table_updated.white;
            table.turn = table_updated.turn;

        });

    });

    req.on('error', function response(error) {
        console.log(error);
    });

    req.write(data);
    req.end();

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
