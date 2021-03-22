// entry point for command line tool

import * as path from 'path';
import {addExportBuildStep} from "./exporters";
import {Project} from "./project";
import {rebuild_ekc} from "./ekc";
import {increaseProjectVersion} from "./version";
import {UtilityConfig} from "./utils";
import * as fs from "fs";

console.debug("Arguments:", process.argv);
console.debug("Working directory:", process.cwd());

if(process.argv.indexOf("--verbose") >= 0 || process.argv.indexOf("-v") >= 0) {
    UtilityConfig.verbose = true;
}

if (process.argv.indexOf("rebuild-ekc") >= 0) {
    rebuild_ekc().then();
} else {
    defaultRun();
}

function defaultRun() {
    const project = new Project();
    project.path.dump();

    console.log("=== EK PROJECT ===");
    console.log("Current Target:", project.current_target);
    console.log("Arguments:", project.args);

    if (project.options.clean) {
        const dir = path.join(process.cwd(), "export");
        if(fs.existsSync(dir)) {
            console.info("Clean: remove EXPORT directory", dir)
            fs.rmSync(dir, {recursive: true});
        }
        else {
            console.log("Clean: nothing to remove", dir)
        }
    }

    if (project.options.increaseVersion !== undefined) {
        increaseProjectVersion(process.cwd(), project.options.increaseVersion);
    }

    addExportBuildStep(project);

    project.importModule("@ekx/miniaudio", project.path.EKX_ROOT);
    project.importModule("@ekx/stb", project.path.EKX_ROOT);
    project.importModule("@ekx/sokol", project.path.EKX_ROOT);
    project.importModule("@ekx/tracy", project.path.EKX_ROOT);
    project.includeProject(path.join(project.path.EKX_ROOT, "ek"));
    project.includeProject(path.join(project.path.EKX_ROOT, "core"));
    project.includeProject(path.join(project.path.EKX_ROOT, "scenex"));
    project.includeProject(process.cwd());

    project.runBuildSteps();
}