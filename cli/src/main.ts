// entry point for command line tool

import * as path from 'path';
import {addExportBuildStep} from "./exporters";
import {Project} from "./project";
import {rebuild_ekc} from "./ekc";

console.debug("Arguments:", process.argv);
console.debug("Working directory:", process.cwd());

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

    if(project.args.indexOf("-o") >= 0) {
        project.options.openProject = true;
    }
    else if(project.args.indexOf("do-not-open") < 0) {
        project.options.openProject = false;
    }

    addExportBuildStep(project);

    project.includeProject(path.join(project.path.EKX_ROOT, "external/tracy"));
    project.includeProject(path.join(project.path.EKX_ROOT, "external/miniaudio"));
    project.includeProject(path.join(project.path.EKX_ROOT, "external/stb"));
    project.includeProject(path.join(project.path.EKX_ROOT, "external/sokol"));
    project.includeProject(path.join(project.path.EKX_ROOT, "ek"));
    project.includeProject(path.join(project.path.EKX_ROOT, "core"));
    project.includeProject(path.join(project.path.EKX_ROOT, "scenex"));
    project.includeProject(process.cwd());

    project.runBuildSteps();
}