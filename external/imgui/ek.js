/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "imgui",
        path: __dirname,
        cpp: ["src"]
    });
    project.importModule("@ekx/freetype", __dirname);
    project.importModule("@ekx/stb", __dirname);
}

module.exports = setup;