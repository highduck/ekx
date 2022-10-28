import {Project} from "../../modules/cli/project.js";

export function setup(project:Project) {
    project.addModule({
        name: "quickjs",
        cpp: "src",
        cpp_define: ['CONFIG_VERSION="1"', "_GNU_SOURCE"],
    });
}
