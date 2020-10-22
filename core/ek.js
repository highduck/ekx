module.exports = (ctx) => {
    ctx.modules.push({
        name: "core",
        cpp: [__dirname]
    });
};