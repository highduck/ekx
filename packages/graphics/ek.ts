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
    await project.importModule("@ekx/ekx/packages/core");
    await project.importModule("@ekx/ekx/packages/app");
    await project.importModule("@ekx/ekx/external/sokol");
}
