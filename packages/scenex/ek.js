/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "scenex",
        path: __dirname,
        cpp: "src",
        cpp_include: "include",
        android: {
            android_permission: "android.permission.INTERNET"
        },
    });
    project.importModule("@ekx/sg-file", __dirname);
    project.importModule("@ekx/graphics", __dirname);
    project.importModule("@ekx/audio", __dirname);
    project.importModule("@ekx/local-storage", __dirname);
    project.importModule("@ekx/texture-loader", __dirname);
    project.importModule("@ekx/res", __dirname);
    project.importModule("@ekx/ecs", __dirname);
}

module.exports = setup;