/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "dev-fonts",
        path: __dirname,
        cpp: ["src"]
    });
}

module.exports = setup;