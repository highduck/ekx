/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "texture-loader",
        path: __dirname,
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
    project.importModule("@ekx/core", __dirname);
    project.importModule("@ekx/app", __dirname);
    project.importModule("@ekx/sokol", __dirname);
}

module.exports = setup;