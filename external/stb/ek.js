/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "stb",
        path: __dirname,
        cpp_include: ["include"]
    });
}

module.exports = setup;