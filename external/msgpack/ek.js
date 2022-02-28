/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "msgpack",
        path: __dirname,
        cpp_include: "include"
    });
}

module.exports = setup;