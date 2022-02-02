/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "quickjs",
        path: __dirname,
        cpp: "src",
        cpp_define: ['CONFIG_VERSION="1"', "_GNU_SOURCE"],
    });
}

module.exports = setup;