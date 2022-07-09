/**
 *
 * @param {Project} project
 */
export function setup(project) {
    project.addModule({
        name: "msgpack",
        cpp_include: "include"
    });
}
