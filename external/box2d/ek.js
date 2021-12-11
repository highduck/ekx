/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "box2d",
        path: __dirname,
        cpp: ["src", "include"]
    });
}

module.exports = setup;