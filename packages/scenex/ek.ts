import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "scenex",
        cpp: "src",
        cpp_include: "include",
        android: {
            android_permission: "android.permission.INTERNET"
        },
    });
    await project.importModule("@ekx/ekx/packages/sg-file");
    await project.importModule("@ekx/ekx/packages/graphics");
    await project.importModule("@ekx/ekx/packages/audio");
    await project.importModule("@ekx/ekx/packages/local-storage");
    await project.importModule("@ekx/ekx/packages/texture-loader");
    await project.importModule("@ekx/ekx/packages/res");
    await project.importModule("@ekx/ekx/packages/ecs");
}
