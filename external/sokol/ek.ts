import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "sokol",
        cpp_include: "include"
    });
}
