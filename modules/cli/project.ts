import * as fs from "fs";
import * as path from "path";
import {BumpVersionFlag, SemVer} from "./version.js";
import {resolveFrom} from "./utility/resolveFrom.js";
import {ModuleDef, validateModuleDef} from "./module.js";
import {logger} from "./logger.js";
import {makeDirs} from "./utils.js";
import {getModuleDir, readTextFileSync, writeTextFileSync} from "../utils/utils.js";

const __dirname = getModuleDir(import.meta);

class ProjectSDK {
    templates = path.resolve(__dirname, "_templates");
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
    args: string[] = process.argv.slice(2);

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
    appIcon?: string;

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
    readonly web: {
        firebaseConfig?: object,
        firebaseToken?: string,
        applications?: { platform: string, url: string, id?: string }[],
        // html colors
        background_color?: string,
        text_color?: string,
        // open-graph object
        og?: {
            title?: string,
            description?: string,
            url?: string,
            image?: string
        },
        headCode: string[],
        bodyHTML: string[],
        bodyScript: string[],
    } = {
        background_color: "#222222",
        text_color: "#CCCCCC",
        headCode: [],
        bodyHTML: [],
        bodyScript: [],
    };

    addModule(def: ModuleDef) {
        if (def.path == null) {
            def.path = this.__dirname;
        }
        validateModuleDef(def);
        this.modules.push(def);
    }

    async loadModule(configPath: string) {
        if(!fs.existsSync(configPath)) {
            console.error("File not found:", configPath);
        }
        configPath = fs.realpathSync(configPath);
        if (this.projects[configPath]) {
            return;
        }
        try {
            logger.log(`Loading module from "${configPath}"`);
            const module = await import(configPath);
            const configurator = module.setup;
            if (configurator) {
                const prev = this.__dirname;
                this.__dirname = path.dirname(configPath);
                {
                    const task = configurator(this) ?? true;
                    if (task instanceof Promise) {
                        this.projects[configPath] = await task ?? true;
                    } else {
                        this.projects[configPath] = task;
                    }
                }
                this.__dirname = prev;
            }
        } catch (err) {
            logger.error("Error: Module configuration", configPath, err);
        }
    }

    async importModule(moduleId: string, fromDir?: string) {
        const currentDir = fromDir ?? this.__dirname ?? process.cwd();
        const moduleConfigPath = resolveFrom(currentDir, moduleId);
        if (moduleConfigPath) {
            await this.loadModule(moduleConfigPath);
        } else {
            throw Error(`Error: Module "${moduleId}" is not resolved from location "${currentDir}"`);
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
            this.projectPkg = JSON.parse(readTextFileSync(path.join(this.projectPath, "package.json")));
        } catch {
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
        const headerFile = "src/config/build_info.h";
        const content = `
#ifndef APP_BUILD_INFO_H
#define APP_BUILD_INFO_H

#define APP_VERSION_NAME "${this.version.shortName()}" 
#define APP_VERSION_CODE "${this.version.buildNumber()}" 

#endif // APP_BUILD_INFO_H
`;
        const filepath = path.join(this.projectPath, headerFile);
        makeDirs(path.dirname(filepath));
        writeTextFileSync(filepath, content);
    }

    __dirname = "";
}