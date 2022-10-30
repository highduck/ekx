import {Project} from "../../modules/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "local-storage",
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
    await project.importModule("../app/ek.ts");
    // for base64 and others
    await project.importModule("../std/ek.ts");
}
