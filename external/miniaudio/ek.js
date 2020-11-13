const path = require('path');

module.exports = (ctx) => {
    ctx.modules = [
        {
            name: "miniaudio",
            cpp: [path.join(__dirname, "include")]
        }
    ];
};