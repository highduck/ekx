/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "pocketmod",
        cpp_include: "include"
    });
}
