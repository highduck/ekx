import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "std",
        cpp: "src",
        cpp_include: "include",
        web: {
            //cpp: "src-wasm",
            //cpp_lib: "--import-memory"
        }
    });
    await project.importModule("@ekx/ekx/external/stb");
    await project.importModule("@ekx/ekx/external/sokol");
}
