import {Project} from "../../modules/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "@ekx/res",
        cpp: "src",
        cpp_include: "include",
        web: {
            js: "src"
        }
    });
    await project.importModule("../app/ek.ts");
}
