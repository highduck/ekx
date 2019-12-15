module.exports = class File {
    constructor(ctx) {
        ctx.modules.push({
            name: "scenex",
            cpp: [__dirname]
        });
    }
};