import {Project} from "@ekx/ekx/modules/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "sample-1-app",
        cpp: "src"
    });

    project.title = "sample-1-app";
    project.desc = "sample-1-app";
    project.orientation = "portrait";
    await project.importModule("@ekx/ekx/packages/app/ek.ts");
}
