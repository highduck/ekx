/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "ecs",
        cpp: "src"
    });
    await project.importModule("@ekx/core");
}
