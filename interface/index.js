function connectWebSocket() {

    let hostname = window.location.hostname; // サーバ情報が取れるはず
    console.log(hostname);
    let ws = new WebSocket(`ws://${hostname}:8080/`);

    ws.onopen = function (event) {
        console.log('WebSocketに接続しました');
    };

    ws.onerror = function (event) {

        console.log('WebSocketに接続できませんでした');
        console.log(event);
        console.log('再接続を試みます');
        ws.close();
        setTimeout(connectWebSocket, 1000);

    };

    ws.onmessage = function (event) {

        let status = JSON.parse(event.data);
        console.log(status);

        if (status.field.seat) {

            if (status.user.player) {
                document.getElementById('seat').textContent = '席を離れる';
            } else {
                document.getElementById('seat').textContent = '他の人が着席中';
            }

        } else {

            document.getElementById('seat').textContent = '席に座る';

        }

        drawPanel(status);

    }

    return ws;
};

function to2From16(str) {

    const move16bit = ('0'.repeat(16) + str).slice(-16);
    const move2bit = move16bit.split('').map((bits) => {
        const move2bits = parseInt(bits, 16).toString(2);
        return ('0000' + move2bits).slice(-4);
    });
    const bits = move2bit.join('');
    return bits;

}

function drawCanvas(panel, data) {

    const width = panel.clientWidth;
    const height = panel.clientHeight;

    const canvas = document.createElement('canvas');
    canvas.setAttribute('width', width.toString());
    canvas.setAttribute('height', height.toString());

    const ctx = canvas.getContext('2d');
    ctx.beginPath();
    data.rate.forEach(p => {
        ctx.moveTo(0, height / 2);
        p.forEach((r, i) => {
            ctx.lineTo(i + 1, height - r*height);
        });
    });
    ctx.stroke();

    panel.appendChild(canvas);

}

function drawPanel(status) {

    let board = document.getElementById('board');
    let black = status.field.black;
    let white = status.field.white;

    let panels = board.querySelectorAll('div');
    // nodeListには謎なNodeもいる

    panels.forEach((panel, i) => {

        panel.setAttribute('id', `panel_${i.toString().padStart(2, '0')}`);

        while (panel.firstChild) {
            panel.removeChild(panel.firstChild);
        };

        let src = 'panel.png';
        if (black[i] === '1' && white[i] === '1') {
            console.log('Error: duplicate stone.');
        }
        if (black[i] === '1') {
            src = 'black.png';
        }
        if (white[i] === '1') {
            src = 'white.png'
        }
        let img = document.createElement('img');
        img.setAttribute('src', src);
        img.setAttribute('onclick', 'move(this);')
        panel.appendChild(img);

    });

    let search = status.field.search;

    let moves = [];

    search.forEach((p, i) => {
        p.forEach(m => {

            const move = moves.find((move) => {
                return move.move == m.move;
            })
            if (move) {
                move.rate.push(m.rate);
            } else {
                m.rate = [ m.rate ];
                moves.push(m);
            }

        });
    });

    moves.forEach(move => {

        const m = to2From16(move.move);
        let panel = panels[m.indexOf('1')];
        while (panel.firstChild) {
            panel.removeChild(panel.firstChild);
        };
        drawCanvas(panel, move);

    });

}

function drawBoard() {

    let board = document.getElementById('board');
    let template;
    let clone;

    for (let i = 0; i < 64; i++) {
        template = document.getElementById('panel');
        clone = document.importNode(template.content, true);
        //clone.setAttribute('id', `panel_${i.toString().padStart(2, '0')}`);
        //clone.setAttribute('onclick', "move(this);");
        board.appendChild(clone);
    }

}

function takeSeat() {
    ws.send(JSON.stringify({
        key: 'seat'
    }));
}

function move(panel) {
    let point = panel.parentNode.getAttribute('id');
    point = parseInt(point.slice(-2));
    ws.send(JSON.stringify({
        key: 'move',
        value: point
    }));
}

function search() {
    ws.send(JSON.stringify({
        key: 'search'
    }));
}

const ws = connectWebSocket();