/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "physics-arcade",
        path: __dirname,
        cpp: "src"
    });
    project.importModule("@ekx/ecs", __dirname);
}

module.exports = setup;