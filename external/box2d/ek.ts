import {Project} from "../../modules/cli/project.js";

export function setup(project:Project) {
    project.addModule({
        name: "box2d",
        cpp: "src",
        cpp_include: "include"
    });
}
