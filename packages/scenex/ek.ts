import {Project} from "../../modules/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "scenex",
        cpp: "src",
        cpp_include: "include",
        android: {
            android_permission: "android.permission.INTERNET"
        },
    });
    await project.importModule("../sg-file/ek.ts");
    await project.importModule("../graphics/ek.ts");
    await project.importModule("../audio/ek.ts");
    await project.importModule("../local-storage/ek.ts");
    await project.importModule("../texture-loader/ek.ts");
    await project.importModule("../res/ek.ts");
    await project.importModule("../ecs/ek.ts");
}
