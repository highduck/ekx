/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "app",
        cpp: "src",
        cpp_include: "include",
        android: {
            cpp_lib: ["log", "android", "GLESv2"],
            android_java: "java",
            android_dependency: [
                `implementation 'androidx.appcompat:appcompat:1.4.2'`
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
   await project.importModule("@ekx/std");
}
