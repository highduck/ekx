
/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "appbox",
        path: __dirname,
        cpp: "src"
    });

    project.importModule("@ekx/ekx/scenex", __dirname);

    project.importModule("@ekx/plugin-firebase", __dirname);
    project.importModule("@ekx/plugin-admob", __dirname);
    project.importModule("@ekx/plugin-billing", __dirname);
    project.importModule("@ekx/plugin-game-services", __dirname);
}

module.exports = setup;