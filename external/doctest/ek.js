/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "doctest",
        path: __dirname,
        cpp: ["doctest"]
    });
}

module.exports = setup;