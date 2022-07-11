import {Project} from "../../modules/cli/project.ts";

export function setup(project:Project) {
    project.addModule({
        name: "dev-fonts",
        cpp_include: "include"
    });
}
