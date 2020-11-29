const path = require('path');

module.exports = (ctx) => {
    ctx.addModule({
        name: "physics-arcade",
        cpp: [path.join(__dirname, "src")]
    });
};