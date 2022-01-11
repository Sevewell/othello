const fs = require('fs');
const { exec } = require('child_process');
const process = require('process');
const Computer = require('./search');

let server = undefined;
let port;

if (process.env.CERT == 'true') {
    const https = require('https');
    const options = {
        key: fs.readFileSync('cert/key.pem'),
        cert: fs.readFileSync('cert/cert.pem')
    };
    server = https.createServer(options);
    port = 443;
} else {
    const http = require('http');
    server = http.createServer((req, res) => {

        console.log(`Request URI is ${req.url}`);

        let data = ''

        req.on('data', (chunk) => {
            data += chunk;
        });

        req.on('end', () => {

            console.log(`Request Body is ${data}`);

            switch (req.url) {

                case '/move':
                    data = JSON.parse(data);
                    putStone(res, data);
                    break;

                case '/search':
                    data = JSON.parse(data);
                    com.search(data, res);
                    break;

            }

        });

    });
    port = 80;
}

const com = new Computer();
com.process = parseInt(process.env.PROCESS);

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

function to2From16(str) {

    const move16bit = ('0'.repeat(16) + str).slice(-16);
    const move2bit = move16bit.split('').map((bits) => {
        const move2bits = parseInt(bits, 16).toString(2);
        return ('0000' + move2bits).slice(-4);
    });
    const bits = move2bit.join('');
    return bits;

}

server.listen(port);
