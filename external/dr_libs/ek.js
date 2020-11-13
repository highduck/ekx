const path = require('path');

module.exports = (ctx) => {
    ctx.modules = [
        {
            name: "dr_libs",
            cpp: [path.join(__dirname, "include")]
        }
    ];
};