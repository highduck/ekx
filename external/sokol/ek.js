const path = require('path');

module.exports = (ctx) => {
    ctx.addModule({
        name: "sokol",
        cpp: [path.join(__dirname, "include")]
    });
};