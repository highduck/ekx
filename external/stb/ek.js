/**
 *
 * @param {Project} project
 */
export function setup(project) {
    project.addModule({
        name: "stb",
        cpp_include: ["include"]
    });
}
