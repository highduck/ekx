import {Project} from "../../modules/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "ecs",
        cpp: "src"
    });
    await project.importModule("../core/ek.ts");
}
