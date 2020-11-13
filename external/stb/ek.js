const path = require('path');

module.exports = (ctx) => {
    ctx.addModule({
        name: "stb",
        cpp: [path.join(__dirname, "include")]
    });
};