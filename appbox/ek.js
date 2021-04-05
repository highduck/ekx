const path = require('path');

module.exports = (project) => {
    project.addModule({
        name: "appbox",
        cpp: [path.join(__dirname, "src")]
    });

    project.importModule("@ekx/plugin-admob", __dirname);
    project.importModule("@ekx/plugin-billing", __dirname);
    project.importModule("@ekx/plugin-game-services", __dirname);
};