import {Project} from "../../modules/cli/project.ts";

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
    await project.importModule("@ekx/ekx/packages/app");
    // for base64 and others
    await project.importModule("@ekx/ekx/packages/std");
}
