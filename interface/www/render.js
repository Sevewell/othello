export function renderBoard(canvas, ctx) {

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    ctx.fillStyle = 'green';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.strokeStyle = 'black';
    ctx.strokeRect(0, 0, canvas.width, canvas.height);

    const width = Math.floor(canvas.width / 8);
    const height = Math.floor(canvas.height / 8);

    ctx.beginPath();
    for (let c = 1; c < 8; c++) {
        let width_column = width * c;
        ctx.moveTo(width_column, 0);
        ctx.lineTo(width_column, canvas.height);
    }
    for (let r = 1; r < 8; r++) {
        let width_height = height * r;
        ctx.moveTo(0, width_height);
        ctx.lineTo(canvas.width, width_height);
    }
    ctx.closePath();
    ctx.stroke();

}

export function renderStone(canvas, ctx, black, white) {

    const cell_width = Math.floor(canvas.width / 8);
    const cell_height = Math.floor(canvas.height / 8);

    for (let r = 0; r < 8; r++) {

        let relative_y = r * cell_height;        

        for (let c = 0; c < 8; c++) {

            let relative_x = c * cell_width;

            let x = relative_x + cell_width / 2;
            let y = relative_y + cell_height / 2;
            let radius = (cell_width + cell_height) / 4;
            let start_angle = 0 * Math.PI / 180;
            let end_angle = 360 * Math.PI / 180;
        
            if (black[8 * r + c] === '1') {
                ctx.beginPath();
                ctx.arc(x, y, radius, start_angle, end_angle);
                ctx.fillStyle = 'black';
                ctx.fill();
            }
            if (white[8 * r + c] === '1') {
                ctx.beginPath();
                ctx.arc(x, y, radius, start_angle, end_angle);
                ctx.fillStyle = 'white';
                ctx.fill();
            }
    
        }

    }

}

export function renderComputing(canvas, ctx, rate) {

    const cell_width = Math.floor(canvas.width / 8);
    const cell_height = Math.floor(canvas.height / 8);

    for (let i = 0; i < 64; i++) {

        const move = rate.find((move) => {
            return move.move == i;
        });
        if (move) {

            let c = i % 8;
            let r = Math.floor(i / 8);
            let width = cell_width * c + cell_width / 2;
            let height = cell_height * r + cell_height / 2;

            ctx.fillStyle = 'white';
            ctx.font = '32px Arial';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(move.rate.length.toString(), width, height);

        }
    }

};
