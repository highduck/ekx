import {Project} from "../../modules/cli/project.js";

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
    await project.importModule("../core/ek.ts");
    await project.importModule("../app/ek.ts");
    await project.importModule("../../external/sokol/ek.ts");
}
