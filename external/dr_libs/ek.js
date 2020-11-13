const path = require('path');

module.exports = (ctx) => {
    ctx.addModule({
        name: "dr_libs",
        cpp: [path.join(__dirname, "include")]
    });
};