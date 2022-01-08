import { renderComputing } from './render.js';
import { renderBoard } from '/render.js';

function connectWebSocket() {

    let hostname = window.location.hostname; // サーバ情報が取れるはず
    console.log(hostname);
    if (hostname == 'localhost') {
	    var ws = new WebSocket(`ws://${hostname}/`);
    } else {
        var ws = new WebSocket(`ws://${hostname}/`); // 本当はTLSにする
    }

    ws.onopen = function (event) {

        console.log('WebSocketに接続しました');
        drawPanel(null);

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

        if (status.name == 'computing') {

            renderComputing(canvas, ctx, status.data);

        } else {

            updateTurn(status, 'black');
            updateTurn(status, 'white');
    
            document.getElementById('black_time').textContent = '持ち時間:' + status.black.time;
            document.getElementById('white_time').textContent = '持ち時間:' + status.white.time;
    
            drawPanel(status);
    
        }
    
    }

    return ws;
};

function updateTurn(status, turn) {

    if (status.user[turn]) {

        document.getElementById(turn + '_seat').textContent = '席を離れる';

    } else {

        if (status[turn].player) {
            document.getElementById(turn + '_seat').textContent = '他の人が着席中';
        } else {
            document.getElementById(turn + '_seat').textContent = '席に座る';
        }

    }

}

function updateStone(board_) { //重いので更新部分だけ描画したい

    const update = [];
    for (let i = 0; i < 64; i++) {
        if (board[i] != board_[i]) {
            update.push(board_[i]);   
        } else {
            update.push(null);
        }
    }

    return update;

}

function drawPanel(status) {

    const board = [];

    for (let i = 0; i < 64; i++) {
        board.push(null);
    }

    if (status) {

        for (let i = 0; i < 64; i++) {

            board[i] = null;

            if (status.black.stone[i] == '1') {
                board[i] = 'black';
                continue;
            }
            if (status.white.stone[i] == '1') {
                board[i] = 'white';
                continue;
            }
    
        }
    
    }

    renderBoard(canvas, ctx, board);

}

document.getElementById('black_seat').addEventListener("click", () => {

    ws.send(JSON.stringify({
        key: 'seat',
        value: 'black'
    }));

});
document.getElementById('black_switch').addEventListener("click", () => {

    ws.send(JSON.stringify({
        key: 'switch',
        value: parseFloat(document.getElementById('black_learning_rate').value)
    }));

});
document.getElementById('white_seat').addEventListener("click", () => {

    ws.send(JSON.stringify({
        key: 'seat',
        value: 'white'
    }));

});
document.getElementById('white_switch').addEventListener("click", () => {

    ws.send(JSON.stringify({
        key: 'switch',
        value: parseFloat(document.getElementById('white_learning_rate').value)
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
