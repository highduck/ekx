import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "appbox",
        cpp: "src"
    });

    await project.importModule("../scenex/ek.ts");
    await project.importModule("../../plugins/firebase/ek.ts");
    await project.importModule("../../plugins/admob/ek.ts");
    await project.importModule("../../plugins/billing/ek.ts");
    await project.importModule("../../plugins/game-services/ek.ts");
}
