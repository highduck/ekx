/**
 *
 * @param {Project} project
 */
export function setup(project) {
    project.addModule({
        name: "dr-libs",
        cpp_include: ["include"]
    });
}
