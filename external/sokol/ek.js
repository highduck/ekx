/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "sokol",
        path: __dirname,
        cpp: ["src"]
    });
}

module.exports = setup;