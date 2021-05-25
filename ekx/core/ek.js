const path = require("path");

module.exports = (ctx) => {
    ctx.addModule({
        name: "core",
        cpp: [path.join(__dirname, "src")]
    });
};