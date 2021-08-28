/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "physics-arcade",
        path: __dirname,
        cpp: ["src"]
    });
}

module.exports = setup;