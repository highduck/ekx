import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "pocketmod",
        cpp_include: "include"
    });
}
