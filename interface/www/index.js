import { renderBoard } from '/render.js';
import { renderStone } from '/render.js';
import { renderComputing } from './render.js';

function connectWebSocket() {

    let hostname = window.location.hostname; // サーバ情報が取れるはず
    console.log(hostname);
    if (hostname == 'localhost') {
	var ws = new WebSocket(`ws://${hostname}:8080/`);
    } else {
        var ws = new WebSocket(`wss://${hostname}:8080/`);
    }

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

        if (status.table.seat) {

            if (status.user.player) {
                document.getElementById('seat').textContent = '席を離れる';
            } else {
                document.getElementById('seat').textContent = '他の人が着席中';
            }

        } else {

            document.getElementById('seat').textContent = '席に座る';

        }

        document.getElementById('time').textContent = status.table.time.toString();

        drawPanel(status);
    
    }

    return ws;
};

function drawPanel(status) {

    let black = status.table.black;
    let white = status.table.white;

    renderBoard(canvas, ctx);
    renderStone(canvas, ctx, black, white);
    renderComputing(canvas, ctx, status.computing.search);
    console.log('playout:', status.computing.playout);
    console.log('rate:', status.computing.rate);
    console.log('node:', status.computing.node);

}

document.getElementById('seat').addEventListener("click", () => {

    ws.send(JSON.stringify({
        key: 'seat'
    }));

});

function move(event) {

    const cell_width = Math.floor(canvas.width / 8);
    const cell_height = Math.floor(canvas.height / 8);

    const rect = event.target.getBoundingClientRect();
    const x = (event.clientX - rect.left) * (canvas.width / rect.width);
    const y = (event.clientY  - rect.top) * (canvas.height / rect.height);

    const c = Math.floor(x / cell_width);
    const r = Math.floor(y / cell_height);

    ws.send(JSON.stringify({
        key: 'move',
        value: r * 8 + c
    }));

}

const canvas = document.getElementById("board");
canvas.addEventListener('click', move, false);
const ctx = canvas.getContext('2d');

const ws = connectWebSocket();
