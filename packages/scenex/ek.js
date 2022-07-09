/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "scenex",
        cpp: "src",
        cpp_include: "include",
        android: {
            android_permission: "android.permission.INTERNET"
        },
    });
    await project.importModule("@ekx/sg-file");
    await project.importModule("@ekx/graphics");
    await project.importModule("@ekx/audio");
    await project.importModule("@ekx/local-storage");
    await project.importModule("@ekx/texture-loader");
    await project.importModule("@ekx/res");
    await project.importModule("@ekx/ecs");
}
