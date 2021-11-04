/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "local-storage",
        path: __dirname,
        cpp: "src",
        android: {
            android_java: "platform/android/java"
        },
        web: {
            js: "platform/web"
        }
    });
    // for base app platform
    project.importModule("@ekx/app", __dirname);
    // for base64 and others
    project.importModule("@ekx/core", __dirname);
}

module.exports = setup;