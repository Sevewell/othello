const process = require('process');
const fs = require('fs');
const path = require('path');
const WebSocket = require('ws');

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

const engines = [];

for (let i = 1; i < 20; i++) {
    const hostname = process.env['ENGINE' + i];
    if (hostname) {
        engines.push(hostname);
    };
};

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
        com: {
            rate: null,
            node: null
        }
    },
    white: {
        ws: null,
        time: null,
        move: null,
        com: {
            rate: null,
            node: null
        }
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

    sendResponse();

}

function putStone(ws, point) {

    const data = JSON.stringify({
        table: table,
        move: point
    });

    const options = {
        hostname: engines[0],
        port: 80,
        path: '/move',
        method: 'GET',
        headers: {
            'Content-Type': 'application/json',
            'Content-Length': data.length
        }
    };

    const req = http.request(options, (res) => {

        res.on('data', (d) => {

            table_updated = JSON.parse(d);
            console.log(table_updated);
            table.black = table_updated.black;
            table.white = table_updated.white;
            table.turn = table_updated.turn;

            sendResponse();

        });

    });

    req.on('error', function response(error) {
        console.log(error);
    });

    req.write(data);
    req.end();

}

function requestSearch(hostname, result) {

    const data = JSON.stringify({
        table: table
    });

    const options = {
        hostname: hostname,
        port: 80,
        path: '/search',
        method: 'GET',
        headers: {
            'Content-Type': 'application/json',
            'Content-Length': data.length
        },
        timeout: 60000
    };

    const req = http.request(options, (res) => {

        res.on('data', (d) => {

            const progress = JSON.parse(d);
            result.push(progress);

        });

    });

    req.on('error', function response(error) {
        console.log(error);
    });

    req.write(data);
    req.end();

}

function summarySearch(result, engine) {

    setTimeout(() => {

        if (result.length == engine) {

            const processes = result.reduce((p, c) => {
                c.forEach(process => {
                    p.push(process);
                });
                return p;
            }, []);

            const aggregate = [];

            processes.forEach(p => {

                const move = aggregate.find(m => {
                    return m.point == p.move;
                });

                if (move) {
                    move.count += 1;
                } else {
                    aggregate.push({
                        point: p.move,
                        count: 1
                    });
                }

            });

            summayComputing(processes, aggregate);

            //selectMove(aggregate);

        } else {
            summarySearch(result, engine);
        }

    }, 1000);

}

function summayComputing(processes, options) {

    const rates = processes.map(p => {
        return p.rate;
    });
    const nodes = processes.map(p => {
        return p.node;
    });

    wss.clients.forEach(function each(client) {
        client.send(JSON.stringify({
            name: 'computing',
            data: {
                turn: table.turn,
                option: options,
                rate: rates,
                node: nodes
            }
        }));
    });

}

function selectMove(options) {

    const choice = options.reduce((p, c) => {
        if (c.count > p.count) {
            return c;
        } else {
            return p;
        }
    }, { point: '0', count: 0 });

    console.log(choice);

    putStone({}, choice.point);

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

                    const result = [];

                    engines.forEach(engine => {
                        requestSearch(engine, result);
                    });

                    summarySearch(result, engines.length);

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

function sendResponse() {

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
        },
        computing: {
            rate: player[table.turn].com.rate,
            node: player[table.turn].com.node,
            option: player[table.turn].com.option    
        }

    }

    wss.clients.forEach(function each(client) {
        status.user = client.status;
        client.send(JSON.stringify(status));
    });

}

setInterval(() => {

    player[table.turn].time += 1;

}, 1000);
