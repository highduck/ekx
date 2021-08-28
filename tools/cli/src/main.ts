// entry point for command line tool

import * as path from 'path';
import {addExportBuildStep} from "./exporters";
import {Project} from "./project";
import {increaseProjectVersion} from "./version";
import {UtilityConfig} from "./utils";
import * as fs from "fs";
import {fixMP3} from "./utility/fix-mp3";
import * as process from "process";
import {logger} from "./logger";

logger.info("📺 EKX 📺");

if (process.argv.indexOf("help") >= 0) {
    logger.info("--verbose : enable verbose mode");
    logger.info("--diag : enable special debug output");
}

if (process.argv.indexOf("--verbose") >= 0 || process.argv.indexOf("-v") >= 0) {
    UtilityConfig.verbose = true;
}

if (process.argv.indexOf("--diag") >= 0) {
    logger._diag = true;
}

logger.log("process.argv:", process.argv);
logger.log("process.cwd:", process.cwd());

defaultRun();

function defaultRun() {

    const project = new Project();

    logger.log(project.path);
    logger.log("- current target:", project.current_target);
    logger.log("- arguments:", project.args);

    if (project.args.indexOf("fix-mp3") >= 0) {
        fixMP3(path.join(project.getAssetsInput(), "**/*.mp3")).catch(err => process.exit(-1));
        return;
    }

    if (project.options.clean) {
        const dir = path.join(process.cwd(), "export");
        if (fs.existsSync(dir)) {
            logger.info("🗑 Remove EXPORT directory", dir)
            fs.rmSync(dir, {recursive: true});
        } else {
            logger.log("🗑 Nothing to remove", dir)
        }
    }

    if (project.options.increaseVersion !== undefined) {
        increaseProjectVersion(process.cwd(), project.options.increaseVersion);
    }

    addExportBuildStep(project);

    project.importModule("@ekx/stb", project.path.EKX_ROOT);
    project.importModule("@ekx/sokol", project.path.EKX_ROOT);
    project.importModule("@ekx/tracy", project.path.EKX_ROOT);
    project.importModule("@ekx/ekx/ek", project.path.EKX_ROOT);
    project.importModule("@ekx/ekx/core", project.path.EKX_ROOT);
    project.importModule("@ekx/ekx/sg-file", project.path.EKX_ROOT);
    project.importModule("@ekx/ekx/scenex", project.path.EKX_ROOT);
    project.loadModule(path.resolve(process.cwd(), "ek.js"));

    project.runBuildSteps().then();
}