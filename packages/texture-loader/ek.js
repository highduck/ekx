/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "texture-loader",
        path: __dirname,
        cpp: "src",
        android: {
            android_java: "platform/android/java"
        },
        apple: {
            cpp_flags: {
                files: [
                    "src/ek/TextureLoader.cpp"
                ],
                flags: "-x objective-c++"
            },
        },
        web: {
            js: "platform/web/lib",
            js_pre: "platform/web/pre"
        }
    });
    project.importModule("@ekx/core", __dirname);
    project.importModule("@ekx/app", __dirname);
    project.importModule("@ekx/sokol", __dirname);
}

module.exports = setup;