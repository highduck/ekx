/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "scenex",
        path: __dirname,
        cpp: __dirname
    });
    project.importModule("@ekx/ekx/sg-file", __dirname);
    project.importModule("@ekx/ekx/ek", __dirname);
}

module.exports = setup;