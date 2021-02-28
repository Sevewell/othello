import { renderBoard } from '/render.js';
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

        updateTurn(status, 'black');
        updateTurn(status, 'white');

        document.getElementById('black_time').textContent = '持ち時間：' + status.black.time;
        document.getElementById('white_time').textContent = '持ち時間：' + status.white.time;

        drawPanel(status);
    
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

    if (status[turn].com) {
        document.getElementById(turn + '_switch').textContent = 'コンピュータ';
    } else {
        document.getElementById(turn + '_switch').textContent = '人間';
    }

}

function updateStone(board_) {

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

    const board_ = [];
    const com = status[status.turn].com;

    for (let i = 0; i < 64; i++) {

        if (status.black.stone[i] == '1') {
            board_.push('black');
            continue;
        }
        if (status.white.stone[i] == '1') {
            board_.push('white');
            continue;
        }

        if (com) {
            const move = com.search.find((move) => {
                return move.move == i;
            });
            if (move) {
                board_.push(move.count);
                continue;
            }
        };

        board_.push('plane');

    }

    const update = updateStone(board_);
    board = board_;

    renderBoard(canvas, ctx, update);
    if (com) {
        renderComputing(canvas, ctx, com, status.turn);
    }

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

let board = [];
for (let i = 0; i < 64; i++) {
    board.push(undefined);
}

const canvas = document.getElementById("board");
canvas.addEventListener('click', move, false);
const ctx = canvas.getContext('2d');

const ws = connectWebSocket();
