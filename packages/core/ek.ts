import {Project} from "../../modules/cli/project.ts";

export async function setup(project:Project) {
    project.addModule({
        name: "core",
        cpp: "src",
        cpp_include: "include"
    });
    await project.importModule("../std/ek.ts");
}
