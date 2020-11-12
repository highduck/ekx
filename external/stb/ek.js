const path = require('path');

module.exports = (ctx) => {
    ctx.modules = [
        {
            name: "stb",
            cpp: [path.join(__dirname, "include")]
        }
    ];
};