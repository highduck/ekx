import {Project} from "../../modules/cli/project.ts";

async function setup(project:Project) {
    project.addModule({
        name: "imgui",
        cpp: "src"
    });
    await project.importModule("@ekx/ekx/external/freetype");
    await project.importModule("@ekx/ekx/external/stb");
}
