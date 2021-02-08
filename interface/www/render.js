export function renderBoard(canvas, ctx, update) {

    const cell_width = Math.floor(canvas.width / 8);
    const cell_height = Math.floor(canvas.height / 8);
    let index = 0;

    for (let r = 0; r < 8; r++) {

        let relative_y = r * cell_height;        

        for (let c = 0; c < 8; c++) {

            let relative_x = c * cell_width;

            if (update[index]) {

                ctx.clearRect(relative_x, relative_y, cell_width, cell_height);
                ctx.fillStyle = 'green';
                ctx.fillRect(relative_x, relative_y, cell_width, cell_height);
                ctx.strokeStyle = 'black';
                ctx.strokeRect(relative_x, relative_y, cell_width, cell_height);

                if (update[index] == 'black' || update[index] == 'white') {

                    let x = relative_x + cell_width / 2;
                    let y = relative_y + cell_height / 2;
                    let radius = (cell_width + cell_height) / 4;
                    let start_angle = 0 * Math.PI / 180;
                    let end_angle = 360 * Math.PI / 180;
                
                    ctx.beginPath();
                    ctx.arc(x, y, radius, start_angle, end_angle);
                    ctx.fillStyle = update[index];
                    ctx.fill();
    
                }
                
                if (Number.isInteger(update[index])) {

                    ctx.fillStyle = 'white';
                    ctx.font = '32px Arial';
                    ctx.textAlign = 'center';
                    ctx.textBaseline = 'middle';
                    let width = cell_width * c + cell_width / 2;
                    let height = cell_height * r + cell_height / 2;
                    ctx.fillText(update[index].toString(), width, height);

                }
    
            }

            index += 1;
    
        }

    }

}

export function renderComputing(canvas, ctx, computing) {

    const mean_playout = Math.floor(computing.playout.reduce((sum, p) => {
            return sum + p;
        }, 0) / computing.playout.length);
    const mean_rate = computing.rate.reduce((sum, r) => {
        return sum + r;
    }, 0) / computing.rate.length;
    const mean_node = Math.floor(computing.node.reduce((sum, n) => {
        return sum + n;
    }, 0) / computing.node.length);

    document.getElementById("playout").textContent = 'playout: ' + mean_playout.toLocaleString();
    document.getElementById("rate").textContent = 'rate: ' + mean_rate.toFixed(3);
    document.getElementById("node").textContent = 'node: ' + mean_node.toLocaleString();

};
