/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "sg-file",
        cpp: "src"
    });
    await project.importModule("@ekx/core");
}
