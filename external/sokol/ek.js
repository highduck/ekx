/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "sokol",
        cpp_include: ["include"]
    });
}
