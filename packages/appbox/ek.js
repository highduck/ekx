
/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "appbox",
        cpp: "src"
    });

    await project.importModule("@ekx/scenex");
    await project.importModule("@ekx/plugin-firebase");
    await project.importModule("@ekx/plugin-admob");
    await project.importModule("@ekx/plugin-billing");
    await project.importModule("@ekx/plugin-game-services");
}
