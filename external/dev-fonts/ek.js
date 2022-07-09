/**
 *
 * @param {Project} project
 */
export function setup(project) {
    project.addModule({
        name: "dev-fonts",
        cpp_include: ["include"]
    });
}
