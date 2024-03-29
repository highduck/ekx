import {Project} from "../../modules/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "imgui",
        cpp: "src"
    });
    await project.importModule("../freetype/ek.ts");
    await project.importModule("../headers/ek.ts");
}
