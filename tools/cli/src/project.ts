import * as path from "path";
import {VERSION_INDEX_CODE, VERSION_INDEX_MAJOR, VERSION_INDEX_MINOR, VERSION_INDEX_PATCH} from "./version";
import {resolveFrom} from "./utility/resolveFrom";
import {ModuleDef, validateModuleDef} from "./module";
import {logger} from "./logger";

class ProjectPath {
    ekc = path.dirname(resolveFrom(__dirname, "@ekx/ekc/package.json"));
    EKX_ROOT = path.dirname(resolveFrom(__dirname, "@ekx/ekx/package.json"));
    CURRENT_PROJECT_DIR = process.cwd();
    OUTPUT = path.join(process.cwd(), "build");

    cli = path.resolve(__dirname, '..');
    templates = path.resolve(__dirname, '../templates');
}

type RegisteredProject = any;

export class Project {
    readonly path = new ProjectPath();
    current_target = process.argv[2];
    args = process.argv.slice(3);

    // options evaluated from arguments
    options: {
        // open export project
        clean?: boolean,
        openProject?: boolean,
        deploy?: string,
        run?: string,
        increaseVersion?: number,
    } = {};

    name: string;
    version_name: string;
    version_code: string;
    title?: string;
    desc?: string;
    binary_name?: string;
    pwa_url?: string; // empty string by defualt
    cmake_target?: string;
    build_dir?: string; // build
    orientation: "landscape" | "portrait";

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
        return this.assets?.output ?? "export/contents/assets";
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
        game_services_id?: string
    };

    ios: {
        application_id?: string,
        googleServicesConfigDir?: string,
        // path to JSON with app-store credentials, used for fastlane automation
        appStoreCredentials?: string,

        admob_app_id?: string
    } = {};

    web: {
        firebaseToken?: string,
        applications?: { platform: string, url: string, id?: string }[]
    } = {};

    html: {
        firebaseAutoSetup?: boolean,
        google_analytics_property_id?: string,
        // css color
        background_color?: string,
        text_color?: string,
        // open-graph object
        og?: {
            title?: string,
            description?: string,
            url?: string,
            image?: string
        },
    } = {};

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
        } catch {
            logger.error("Module is not resolved", configPath);
        }
    }

    importModule(moduleId: string, fromDir?: string) {
        fromDir = fromDir ?? process.cwd();
        const moduleConfigPath = resolveFrom(fromDir, moduleId + "/ek.js");
        if (moduleConfigPath != null) {
            this.loadModule(moduleConfigPath);
        }
        else {
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
                this.options.increaseVersion = VERSION_INDEX_CODE;
                if (i + 1 < this.args.length) {
                    const second = this.args[i + 1];
                    if (second == "patch") {
                        this.options.increaseVersion = VERSION_INDEX_PATCH;
                    } else if (second == "minor") {
                        this.options.increaseVersion = VERSION_INDEX_MINOR;
                    } else if (second == "major") {
                        this.options.increaseVersion = VERSION_INDEX_MAJOR;
                    } else if (second == "build") {
                        this.options.increaseVersion = VERSION_INDEX_CODE;
                    }
                }
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
                if (this.options.increaseVersion === undefined) {
                    this.options.increaseVersion = VERSION_INDEX_CODE;
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
}