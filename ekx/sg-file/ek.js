const path = require("path");

module.exports = (ctx) => {
    ctx.addModule({
        name: "sg-file",
        cpp: [path.join(__dirname, "src")]
    });
};