import {Project} from "../../modules/cli/project.js";

export function setup(project: Project) {
    project.addModule({
        name: "sokol",
        cpp_include: "include"
    });
}
