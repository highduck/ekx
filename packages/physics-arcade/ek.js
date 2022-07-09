/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "physics-arcade",
        cpp: "src"
    });
    await project.importModule("@ekx/ecs");
}
