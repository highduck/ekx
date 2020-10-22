module.exports = (ctx) => {
    ctx.modules.push({
        name: "scenex",
        cpp: [__dirname]
    });
};