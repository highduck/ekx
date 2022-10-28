import {Project} from "../../modules/cli/project.js";

export function setup(project: Project) {
    project.addModule({
        name: "headers",
        cpp_include: "include"
    });
}
