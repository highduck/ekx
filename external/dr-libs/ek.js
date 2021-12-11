/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "dr-libs",
        path: __dirname,
        cpp_include: ["include"]
    });
}

module.exports = setup;