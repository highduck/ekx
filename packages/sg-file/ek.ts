import {Project} from "../../modules/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "sg-file",
        cpp: "src"
    });
    await project.importModule("../core/ek.ts");
}
