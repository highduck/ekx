/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "local-storage",
        cpp: "src",
        cpp_include: "include",
        android: {
            android_java: "java"
        },
        web: {
            js: "src"
        }
    });
    // for base app platform
    await project.importModule("@ekx/app");
    // for base64 and others
    await project.importModule("@ekx/std");
}
