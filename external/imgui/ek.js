/**
 *
 * @param {Project} project
 */
async function setup(project) {
    project.addModule({
        name: "imgui",
        cpp: ["src"]
    });
    await project.importModule("@ekx/freetype");
    await project.importModule("@ekx/stb");
}
