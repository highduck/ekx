import * as path from 'path';
import {addExportBuildStep} from "./exporters";
import {Project} from "./project";

console.debug("Arguments", process.argv);

const project = new Project();

project.path.dump();

console.log("=== EK PROJECT ===");
console.log("Current Target:", project.current_target);

addExportBuildStep(project);

project.include_project(path.join(project.path.EKX_ROOT, "external/miniaudio"));
project.include_project(path.join(project.path.EKX_ROOT, "external/dr_libs"));
project.include_project(path.join(project.path.EKX_ROOT, "external/stb"));
project.include_project(path.join(project.path.EKX_ROOT, "ek"));
project.include_project(path.join(project.path.EKX_ROOT, "core"));
project.include_project(path.join(project.path.EKX_ROOT, "scenex"));
project.include_project(process.cwd());

project.runBuildSteps();
