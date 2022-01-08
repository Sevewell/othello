const { exec } = require('child_process');
const { execFile } = require('child_process');

module.exports = class Computer {

    constructor() {
        this.power = false;
        this.process = 0;
        this.moves = [];
        this.playout = [];
        this.rate = [];
        this.node = [];
    }

    to2From16(str) {

        const move16bit = ('0'.repeat(16) + str).slice(-16);
        const move2bit = move16bit.split('').map((bits) => {
            const move2bits = parseInt(bits, 16).toString(2);
            return ('0000' + move2bits).slice(-4);
        });
        const bits = move2bit.join('');
        return bits;
    
    }
    
    move(table, choice) {

        const point = this.to2From16(choice.move).indexOf('1');
    
        this.moves = [];
        this.node = [];
        this.playout = [];
        this.rate = [];

        if (table.turn == 'black') {

            const option = `${table.black} ${table.white} ${point}`;

            exec(`./move ${option}`, (err, stdout, stderr) => {

                const move = JSON.parse(stdout);
                table.black = this.to2From16(move.m);
                table.white = this.to2From16(move.y);
                table.turn = 'white';
    
            });

        }
        if (table.turn == 'white') {

            const option = `${table.white} ${table.black} ${point}`;

            exec(`./move ${option}`, (err, stdout, stderr) => {

                const move = JSON.parse(stdout);
                table.white = this.to2From16(move.m);
                table.black = this.to2From16(move.y);
                table.turn = 'black';
    
            });

        }

    }
    
    spawnSearch(table, record) {

        let m;
        let y;
        if (table.turn == 'black') {
            m = table.black;
            y = table.white;
        }
        if (table.turn == 'white') {
            m = table.white;
            y = table.black;
        }

        const seed = Math.floor(Math.random() * Math.floor(1000)).toString();

        execFile('./search', [m, y, seed, this.learning_rate], (error, stdout, stderr) => {
            
            if (error) { return console.error('ERROR', error); };

            const result = JSON.parse(stdout);
            result.move = this.to2From16(result.move).indexOf('1')
            record.push(result);

        });
        
    }
    
    search(table, res) {

        const record = [];
    
        for (let i = 0; i < this.process; i++) {
            
            this.spawnSearch(table, record);

        }
    
        const intervalID = setInterval(() => {
            
            if (record.length == this.process) {

                clearInterval(intervalID);

                res.writeHead(200, { 'Content-Type': 'application/json'} );
                res.write(JSON.stringify(record));
                res.end();            

            }

        }, 1000);
    
    }
    
};