module.exports = (ctx) => {
    ctx.addModule({
        name: "core",
        cpp: [__dirname]
    });
};