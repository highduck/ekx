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
    project.importModule("@ekx/std", __dirname);
}

module.exports = setup;