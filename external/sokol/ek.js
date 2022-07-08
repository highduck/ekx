/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "sokol",
        path: __dirname,
        cpp_include: ["include"]
    });
}

module.exports = setup;