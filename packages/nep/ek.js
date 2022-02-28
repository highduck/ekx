/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "nep",
        path: __dirname,
        cpp: "src",
        cpp_include: "include",
        android: {
            android_java: "java"
        },
        web: {
            js: "src",
            js_script: "pp/msgpack.min.js"
        }
    });
    // for base app platform
    project.importModule("@ekx/app", __dirname);
    // for base64 and others
    project.importModule("@ekx/std", __dirname);
    project.importModule("@ekx/msgpack", __dirname);
}

module.exports = setup;