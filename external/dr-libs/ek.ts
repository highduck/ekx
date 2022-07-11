import {Project} from "../../modules/cli/project.ts";

export function setup(project:Project) {
    project.addModule({
        name: "dr-libs",
        cpp_include: "include"
    });
}
