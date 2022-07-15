// entry point for command line tool

import {path, fs} from "../deps.ts";
import {addExportBuildStep} from "./exporters.ts";
import {Project} from "./project.ts";
import {bumpProjectVersion} from "./version.ts";
import {UtilityConfig} from "./utils.ts";
import {fixMP3} from "./utility/fix-mp3.ts";
import {logger} from "./logger.ts";

logger.info("📺 EKX 📺");

if (Deno.args.indexOf("help") >= 0) {
    logger.info("--verbose | -v : enable verbose mode");
    logger.info("--diag : enable additional debug output");
    logger.info("-V : enable verbose + additional debug");
}

if (Deno.args.indexOf("--verbose") >= 0 || Deno.args.indexOf("-v") >= 0) {
    UtilityConfig.verbose = true;
}

if (Deno.args.indexOf("--diag") >= 0) {
    logger._diag = true;
}

if (Deno.args.indexOf("-V") >= 0) {
    UtilityConfig.verbose = true;
    logger._diag = true;
}

logger.log("process.argv:", Deno.args);
logger.log("process.cwd:", Deno.cwd());

async function defaultRun() {

    const project = new Project();

    logger.log(project.sdk.templates);
    logger.log("- current target:", project.current_target);
    logger.log("- arguments:", project.args);

    if (project.args.indexOf("fix-mp3") >= 0) {
        fixMP3(path.join(project.getAssetsInput(), "**/*.mp3")).catch(_ => Deno.exit(1));
        return;
    }

    if (project.options.clean) {
        const dir = path.join(Deno.cwd(), "export");
        if (fs.existsSync(dir)) {
            logger.info("🗑 Remove EXPORT directory", dir)
            Deno.removeSync(dir, {recursive: true});
        } else {
            logger.log("🗑 Nothing to remove", dir)
        }
    }

    if (project.options.bumpVersion !== undefined) {
        project.version = bumpProjectVersion(project.projectPath, project.options.bumpVersion);
    }

    addExportBuildStep(project);

    await project.loadModule(path.resolve(Deno.cwd(), "ek.ts"));
    await project.runBuildSteps();
}

defaultRun().catch((e)=>{
    logger.error(e);
    Deno.exit(1);
}).then(()=>{
    Deno.exit(0);
});