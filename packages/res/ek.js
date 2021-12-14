/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "@ekx/res",
        path: __dirname,
        cpp: "src",
        cpp_include: "include",
        web: {
            js: "src"
        }
    });
    project.importModule("@ekx/app", __dirname);
}

module.exports = setup;