/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "core",
        path: __dirname,
        cpp: ["src"]
    });
}

module.exports = setup;