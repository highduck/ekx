/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "local-storage",
        path: __dirname,
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
    project.importModule("@ekx/app", __dirname);
    // for base64 and others
    project.importModule("@ekx/std", __dirname);
}

module.exports = setup;