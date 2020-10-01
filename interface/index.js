function connectWebSocket() {

    let ws = new WebSocket(`ws://${location.hostname.replace('interface', 'engine')}:8080/`);

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

        let black = status.field.black
        let white = status.field.white

        for (let i = 0; i < 64; i++) {

            let img = 'panel.png';
            if (black[i] === '1' && white[i] === '1') {
                console.log('Error: duplicate stone.');
            }
            if (black[i] === '1') {
                img = 'black.png';
            }
            if (white[i] === '1') {
                img = 'white.png'
            }

            let panel = document.getElementById(`panel_${i.toString().padStart(2, '0')}`);
            panel.querySelector('img').setAttribute('src', img);

        }

    }

    return ws;
};

function draw() {

    var board = document.getElementById('board');
    var template;
    var clone;
    var panel;
    for (let i = 0; i < 64; i++) {
        template = document.getElementById('panel');
        clone = document.importNode(template.content, true);
        panel = clone.querySelector('div');
        panel.setAttribute('id', `panel_${i.toString().padStart(2, '0')}`);
        panel.querySelector('img').setAttribute('src', 'panel.png');
        board.appendChild(clone);
    }

}

function takeSeat() {
    ws.send(JSON.stringify({
        key: 'seat'
    }));
}

function move(img) {
    let point = img.parentNode.getAttribute('id');
    point = parseInt(point.slice(-2));
    console.log(point);
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