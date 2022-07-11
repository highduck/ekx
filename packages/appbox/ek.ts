import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "appbox",
        cpp: "src"
    });

    await project.importModule("@ekx/ekx/packages/scenex");
    await project.importModule("@ekx/ekx/plugins/firebase");
    await project.importModule("@ekx/ekx/plugins/admob");
    await project.importModule("@ekx/ekx/plugins/billing");
    await project.importModule("@ekx/ekx/plugins/game-services");
}
