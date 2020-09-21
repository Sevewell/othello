const WebSocket = require('ws');
const { exec } = require('child_process');

const server = new WebSocket.Server({ port: 8080 });

const status = {
    seat: false,
    black: '0'.repeat(28) + '1' + '0'.repeat(6) + '1' + '0'.repeat(28),
    white: '0'.repeat(27) + '1' + '0'.repeat(8) + '1' + '0'.repeat(27)
}

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

function search() {

    // 計算中フラグチェック

    exec(`./search ${status.white} ${status.black} 100`, (error, stdout, stderr) => {

        if (error) {
            console.log(`exec error: ${error}`);
            console.error(`stderr: ${stderr}`);
            return;
        }

        let results = stdout.trim().split('\n').map((result) => {
            let values = result.split(', ');
            return {
                black: values[0],
                white: values[1],
                pass: parseFloat(values[2]),
                rate: parseFloat(values[3])
            }
        });
        let moves = results.reduce(function (moves, currentValue) {
            let move = moves.find((m) => {
                return (m.white == currentValue.white) && (m.black == currentValue.black);
            });
            if (move) {
                move.pass.push(currentValue.pass);
                move.rate.push(currentValue.rate);
            } else {
                moves.push({
                    white: currentValue.white,
                    black: currentValue.black,
                    pass: [ currentValue.pass ],
                    rate: [ currentValue.rate ]
                });
            }
            return moves;
        }, []);
        console.log(moves);

        let move = moves.reduce(function (a, b) {
            let a_sum = a.pass.reduce(function (a_a, a_b) {
                return a_a + a_b;
            }, 0);
            let b_sum = b.pass.reduce(function (b_a, b_b) {
                return b_a + b_b;
            }, 0);
            if (b_sum > a_sum) {
                return b;
            } else {
                return a;
            }
        });
        console.log(move);

        status.black = move.black;
        status.white = move.white;
        server.clients.forEach(function each(client) {
            client.send(JSON.stringify({
                field: status,
                user: client.status
            }));
        });

        // 計算中フラグ解除
    });
    // 計算中フラグ立てる

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
                search();
                break;

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
