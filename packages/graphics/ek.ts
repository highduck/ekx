import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "graphics",
        cpp: "src",
        apple: {
            cpp_flags: {
                files: [
                    "src/ek/ek_gfx.c"
                ],
                flags: "-x objective-c"
            },
        }
    });
    await project.importModule("../core/ek.ts");
    await project.importModule("../app/ek.ts");
    await project.importModule("../../external/sokol/ek.ts");
}
