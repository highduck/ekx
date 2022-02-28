/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "app",
        path: __dirname,
        cpp: "src",
        cpp_include: "include",
        android: {
            cpp_lib: ["log", "android", "GLESv2"],
            android_java: "java",
            android_dependency: [
                `implementation 'androidx.appcompat:appcompat:1.4.1'`
            ]
        },
        macos: {
            xcode_framework: [
                "Cocoa",
                "Metal",
                "MetalKit",
                "QuartzCore",
            ]
        },
        ios: {
            xcode_framework: [
                "Foundation",
                "UIKit",
                "Metal",
                "MetalKit",
                "QuartzCore",
            ]
        },
        web: {
            js: "src/web",
            cpp_lib: ["GLESv2", "GL"]
        },
    });
    project.importModule("@ekx/std", __dirname);
}

module.exports = setup;