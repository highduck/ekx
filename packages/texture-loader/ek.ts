import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
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
    await project.importModule("@ekx/ekx/packages/core");
    await project.importModule("@ekx/ekx/packages/app");
    await project.importModule("@ekx/ekx/external/sokol");
}
