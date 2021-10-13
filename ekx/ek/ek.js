/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "ek",
        path: __dirname,
        cpp: "src",
        android: {
            android_permission: "android.permission.INTERNET",
            android_java: "android/java"
        },
        apple: {
            cpp_flags: {
                files: [
                    "src/ek/graphics/graphics.cpp",
                    "src/ek/audio/audio.cpp"
                ],
                flags: "-x objective-c++"
            },
        },
        ios: {
        },
        web: {
            js: "src/ek/texture_loader",
            js_pre: "web"
        },
        windows: {
        },
        linux: {
        }
    });
    project.importModule("auph", __dirname);
    project.importModule("@ekx/ekx/app", __dirname);
    project.importModule("@ekx/sokol", __dirname);
}

module.exports = setup;