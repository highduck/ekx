/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "core",
        path: __dirname,
        cpp: "src",
        cpp_include: "include"
    });
    project.importModule("@ekx/std", __dirname);
}

module.exports = setup;