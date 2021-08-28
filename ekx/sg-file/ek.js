/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "sg-file",
        path: __dirname,
        cpp: ["src"]
    });
}

module.exports = setup;