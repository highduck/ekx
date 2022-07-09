/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "texture-loader",
        cpp: "src",
        cpp_include: "include",
        android: {
            android_java: "java"
        },
        web: {
            js: "js/lib",
            js_pre: "js/pre"
        }
    });
    await project.importModule("@ekx/core");
    await project.importModule("@ekx/app");
    await project.importModule("@ekx/sokol");
}
