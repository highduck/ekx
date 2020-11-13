const path = require('path');

module.exports = (ctx) => {
    ctx.addModule({
        name: "miniaudio",
        cpp: [path.join(__dirname, "include")]
    });
};