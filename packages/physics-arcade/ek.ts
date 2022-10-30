import {Project} from "../../modules/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "physics-arcade",
        cpp: "src"
    });
    await project.importModule("../ecs/ek.ts");
}
