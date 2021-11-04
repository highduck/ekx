/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "core",
        path: __dirname,
        cpp: "src"
    });
    project.importModule("@ekx/stb", __dirname);
    project.importModule("@ekx/sokol", __dirname);
}

module.exports = setup;