#!/usr/bin/env -S node -r "ts-node/register"

// entry point for command line tool
import * as path from "path";
import * as fs from "fs";
import {addExportBuildStep} from "./exporters.js";
import {Project} from "./project.js";
import {bumpProjectVersion} from "./version.js";
import {UtilityConfig} from "./utils.js";
import {fixMP3} from "./utility/fix-mp3.js";
import {logger} from "./logger.js";
import {init} from "../cmake/npm.js";

logger.info("📺 EKX 📺");

if (process.argv.indexOf("help") >= 0) {
    logger.info("--verbose | -v : enable verbose mode");
    logger.info("--diag : enable additional debug output");
    logger.info("-V : enable verbose + additional debug");
}

if (process.argv.indexOf("--verbose") >= 0 || process.argv.indexOf("-v") >= 0) {
    UtilityConfig.verbose = true;
}

if (process.argv.indexOf("--diag") >= 0) {
    logger._diag = true;
}

if (process.argv.indexOf("-V") >= 0) {
    UtilityConfig.verbose = true;
    logger._diag = true;
}

logger.log("process.argv0:", process.argv0);
logger.log("process.argv:", process.argv);
logger.log("process.cwd:", process.cwd());

if (process.argv.indexOf("cmake") >= 0) {
    UtilityConfig.verbose = true;
    logger._diag = true;
    try {
        init();
        process.exit(0);
    }
    catch(e) {
        logger.error("Error", e);
        process.exit(1);
    }
}


async function defaultRun() {

    const project = new Project();

    logger.log(project.sdk.templates);
    logger.log("- current target:", project.current_target);
    logger.log("- arguments:", project.args);

    if (project.args.indexOf("fix-mp3") >= 0) {
        fixMP3(path.join(project.getAssetsInput(), "**/*.mp3")).catch(_ => process.exit(1));
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

    if (project.options.bumpVersion !== undefined) {
        project.version = bumpProjectVersion(project.projectPath, project.options.bumpVersion);
    }

    addExportBuildStep(project);

    await project.loadModule(path.resolve(process.cwd(), "ek.ts"));
    await project.runBuildSteps();
}

defaultRun().catch((e)=>{
    logger.error(e);
    process.exit(1);
}).then(()=>{
    process.exit(0);
});