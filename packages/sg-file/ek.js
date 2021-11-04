/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "sg-file",
        path: __dirname,
        cpp: "src"
    });
    project.importModule("@ekx/core", __dirname);
}

module.exports = setup;