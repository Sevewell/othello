const WebSocket = require('ws');
const { spawn } = require('child_process');
const readline = require('readline');

const server = new WebSocket.Server({ port: 8080 });

const process = 6;

const status = {
    seat: false,
    black: '0'.repeat(28) + '1' + '0'.repeat(6) + '1' + '0'.repeat(28),
    white: '0'.repeat(27) + '1' + '0'.repeat(8) + '1' + '0'.repeat(27),
    search: [],
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

}

function putStone(ws, point) {

    if (ws.status.player) {
        status.black = status.black.slice(0, point) + '1' + status.black.slice(point + 1);
        status.white = status.white.slice(0, point) + '0' + status.white.slice(point + 1);
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

function streamSearch() {

    setTimeout(() => {

        if (status.computing == 0) {

            // 探索結果
            const moves = [];

            status.search.forEach(process => {
                process.forEach(m => {

                    const move = moves.find((move) => {
                        return move.move == m.move;
                    });
                    if (move) {
                        move.rate.push(m.rate.pop());
                    } else {
                        m.rate = [ m.rate.pop() ];
                        moves.push(m);
                    }
                
                });
            });

            const choice = moves.reduce((max, move) => {
                const sum_max = max.rate.reduce((sum, rate) => {
                    return sum + rate;
                }, 0);
                const sum_move = move.rate.reduce((sum, rate) => {
                    return sum + rate;
                }, 0);
                if (sum_move > sum_max) {
                    return move;
                } else {
                    return max;
                }
            });

            status.black = to2From16(choice.black);
            status.white = to2From16(choice.white);
            status.search = [];

            server.clients.forEach(function each(client) {
                client.send(JSON.stringify({
                    field: status,
                    user: client.status
                }));
            });

        } else {

            server.clients.forEach(function each(client) {
                client.send(JSON.stringify({
                    field: status,
                    user: client.status
                }));
            });
            streamSearch();

        }

    }, 1000);

}

function spawnSearch() {

    // プロセス毎の情報はまとめて送りたい
    const process_move = [];
    status.search.push(process_move);

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

        const move = process_move.find((m) => {
            return m.move == data.move;
        });
    
        if (move) {
            move.rate.push(data.rate);
        } else {
            process_move.push({
                black: data.y,
                white: data.m,
                move: data.move,
                rate: [ data.rate ]
            });
        }
    
    });

}

function search(ws) {

    if (!ws.status.player) {
        return;
    }

    if (status.computing != 0) {
        return;
    }

    for (let i = 0; i < process; i++) {
        spawnSearch();
        status.computing += 1;
    }

    streamSearch();

}

server.on('connection', function connection(ws, req) {

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
                putStone(ws, message.value);
                break;
            case 'search':
                search(ws);
                return;

        }

        server.clients.forEach(function each(client) {
            client.send(JSON.stringify({
                field: status,
                user: client.status
            }));
        });
    
    });

    ws.on('close', function close() {

        ;
        
    })
    
});
