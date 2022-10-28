import {Project} from "../../modules/cli/project.js";

export function setup(project: Project) {
    project.addModule({
        name: "msgpack",
        cpp_include: "include"
    });
}
