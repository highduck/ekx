import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "physics-arcade",
        cpp: "src"
    });
    await project.importModule("@ekx/ekx/packages/ecs");
}
