export function renderBoard(canvas, ctx, board) {

    const cell_width = Math.floor(canvas.width / 8);
    const cell_height = Math.floor(canvas.height / 8);

    board.forEach((cell, index) => {
        
        let column = index % 8;
        let row = Math.floor(index / 8);

        let relative_x = cell_width * column;
        let relative_y = cell_height * row;

        renderCell(ctx, relative_x, relative_y, cell_width, cell_height);
        if (cell) {
            renderDisk(ctx, relative_x, relative_y, cell_width, cell_height, cell);
        }

    });
}

function renderCell(ctx, relative_x, relative_y, cell_width, cell_height) {

    ctx.clearRect(relative_x, relative_y, cell_width, cell_height);
    ctx.fillStyle = 'green';
    ctx.fillRect(relative_x, relative_y, cell_width, cell_height);
    ctx.strokeStyle = 'black';
    ctx.strokeRect(relative_x, relative_y, cell_width, cell_height);

}

function renderDisk(ctx, relative_x, relative_y, cell_width, cell_height, color) {

    let x = relative_x + cell_width / 2;
    let y = relative_y + cell_height / 2;
    let radius = (cell_width + cell_height) / 4;
    let start_angle = 0 * Math.PI / 180;
    let end_angle = 360 * Math.PI / 180;

    ctx.beginPath();
    ctx.arc(x, y, radius, start_angle, end_angle);
    ctx.fillStyle = color;
    ctx.fill();

}

export function renderComputing(canvas, ctx, computing) {

    const median_rate = computing.rate[Math.floor(computing.rate.length / 2)];
    const mean_node = Math.floor(computing.node.reduce((sum, n) => {
        return sum + n;
    }, 0) / computing.node.length);
    document.getElementById("rate").textContent = 'rate: ' + median_rate.toFixed(3);
    document.getElementById("node").textContent = 'node: ' + mean_node.toLocaleString();

    computing.option.forEach(option => {

        const cell_width = Math.floor(canvas.width / 8);
        const cell_height = Math.floor(canvas.height / 8);
        let column = option.point % 8;
        let row = Math.floor(option.point / 8);

        ctx.fillStyle = computing.turn;
        ctx.font = '32px Arial';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';

        let width = cell_width * column + cell_width / 2;
        let height = cell_height * row + cell_height / 2;
        ctx.fillText(option.count.toString(), width, height);

    });

};
