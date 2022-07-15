import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "ecs",
        cpp: "src"
    });
    await project.importModule("../core/ek.ts");
}
