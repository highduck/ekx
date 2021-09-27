import * as path from "path";
import {BumpVersionFlag, SemVer} from "./version";
import {resolveFrom} from "./utility/resolveFrom";
import {ModuleDef, validateModuleDef} from "./module";
import {logger} from "./logger";
import * as fs from "fs";
import {makeDirs} from "./utils";

class ProjectSDK {
    EKX_ROOT = path.dirname(resolveFrom(__dirname, "@ekx/ekx/package.json"));
    templates = path.resolve(__dirname, '../templates');
}

type RegisteredProject = any;

export class Project {
    readonly sdk = new ProjectSDK();
    // current project's path, default is cwd()
    projectPath: string = process.cwd();
    projectPkg: any = undefined;

    // code-name for project, initially initialized from project's package.json,
    // if not available, "unnamed", you can change it in app project config: project.name = "my-project"
    name: string;

    // loaded from project's path package.json
    version: SemVer;

    current_target: string = process.argv[2];
    args: string[] = process.argv.slice(3);

    // options evaluated from arguments
    options: {
        // open export project
        clean?: boolean,
        openProject?: boolean,
        deploy?: string,
        run?: string,
        bumpVersion?: BumpVersionFlag,
    } = {};

    title?: string;
    desc?: string;
    build_dir?: string; // build
    orientation: "landscape" | "portrait" = "portrait";

    build_steps: (() => void | Promise<any>)[] = [];
    projects: { [name: string]: RegisteredProject } = {};
    modules: ModuleDef[] = [];

    assets?: {
        input?: string;
        output?: string;
    };

    getAssetsInput(): string {
        return this.assets?.input ?? "assets";
    }

    getAssetsOutput(): string {
        return this.assets?.output ?? "export/content/assets";
    }

    market_asset?: string;

    android: {
        // android application package name
        application_id?: string,
        // just package for java code
        package_id?: string,
        googleServicesConfigDir?: string,
        signingConfigPath?: string,
        // path to service account api key json file, used for fastlane automation
        serviceAccountKey?: string,

        admob_app_id?: string,
        game_services_id?: string,
    } = {};

    ios: {
        application_id?: string,
        googleServicesConfigDir?: string,
        // path to JSON with app-store credentials, used for fastlane automation
        appStoreCredentials?: string,

        admob_app_id?: string,
    } = {};

    onProjectGenerated: (() => void)[] = [];
    web: {
        firebaseToken?: string,
        applications?: { platform: string, url: string, id?: string }[]
    } = {};

    html: {
        // css color
        background_color?: string,
        text_color?: string,
        // firebase analytics
        firebaseAutoSetup?: boolean,
        google_analytics_property_id?: string,
        // open-graph object
        og?: {
            title?: string,
            description?: string,
            url?: string,
            image?: string
        },
    } = {
        background_color: "#222222",
        text_color: "#CCCCCC",
    };

    addModule(def: ModuleDef) {
        validateModuleDef(def);
        this.modules.push(def);
    }

    loadModule(configPath: string) {
        if (this.projects[configPath]) {
            return;
        }
        try {
            logger.log(`Loading module from "${configPath}"`);
            const configurator = require(configPath);
            if (configurator) {
                this.projects[configPath] = configurator(this);
            }
        } catch (err) {
            logger.error("Module is not resolved", configPath);
            logger.error(err);
        }
    }

    importModule(moduleId: string, fromDir?: string) {
        fromDir = fromDir ?? process.cwd();
        const moduleConfigPath = resolveFrom(fromDir, moduleId + "/ek.js");
        if (moduleConfigPath != null) {
            this.loadModule(moduleConfigPath);
        } else {
            logger.warn(`ek.js module not found for "${moduleId}" from dir "${fromDir}"`);
        }
    }

    async runBuildSteps() {
        for (const step of this.build_steps) {
            const res = step();
            if (res instanceof Promise) {
                await res;
            }
        }
    }

    constructor() {
        try {
            this.projectPkg = JSON.parse(fs.readFileSync(path.join(this.projectPath, "package.json"), "utf8"));
        } catch (e) {
            logger.warn("Unable to read project's package.json file");
            this.projectPkg = {};
        }

        this.name = this.projectPkg.name ?? "unnamed";
        try {
            this.version = SemVer.parse(this.projectPkg.version);
        } catch (e) {
            logger.warn("Unable to parse version from project's package.json");
            this.version = new SemVer(1, 0, 0);
        }

        if (this.args.indexOf("clean") >= 0) {
            this.options.clean = true;
        }

        if (this.args.indexOf("-o") >= 0) {
            this.options.openProject = true;
        } else if (this.args.indexOf("do-not-open") < 0) {
            this.options.openProject = false;
        }

        {
            const i = this.args.indexOf("--bump");
            if (i >= 0) {
                this.options.bumpVersion = SemVer.parseBump(this.args[i + 1], BumpVersionFlag.BuildNumber);
            }
        }
        {
            const i = this.args.indexOf("--deploy");
            if (i >= 0) {
                this.options.deploy = "internal";
                if (i + 1 < this.args.length) {
                    const second = this.args[i + 1];
                    if (second === "beta" || second === "alpha" || second === "production") {
                        this.options.deploy = second;
                    }
                }
                if (this.options.bumpVersion === undefined) {
                    this.options.bumpVersion = BumpVersionFlag.BuildNumber;
                }
            }
        }

        {
            const i = this.args.indexOf("--run");
            if (i >= 0) {
                this.options.run = "wip";
            }
        }
    }

    generateNativeBuildInfo() {
        const headerFile = "src/config/BuildInfo.h";
        const content = `
#pragma once
namespace AppVersion {
const char* Name = "${this.version.shortName()}";
const char* Code = "${this.version.buildNumber()}";
}
`;
        const filepath = path.join(this.projectPath, headerFile);
        makeDirs(path.dirname(filepath));
        fs.writeFileSync(filepath, content);
    }
}