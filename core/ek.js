module.exports = class File {
    constructor(ctx) {
        ctx.modules.push({
            name: "core",
            cpp: [__dirname]
        });
    }
};