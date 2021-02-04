import * as path from 'path';
import {addExportBuildStep} from "./exporters";
import {Project} from "./project";

console.debug("Arguments", process.argv);

const project = new Project();

project.path.dump();

console.log("=== EK PROJECT ===");
console.log("Current Target:", project.current_target);

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
