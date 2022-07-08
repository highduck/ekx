/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "dev-fonts",
        path: __dirname,
        cpp_include: ["include"]
    });
}

module.exports = setup;