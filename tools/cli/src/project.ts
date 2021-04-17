import * as path from "path";
import {VERSION_INDEX_CODE, VERSION_INDEX_MAJOR, VERSION_INDEX_MINOR, VERSION_INDEX_PATCH} from "./version";
import {resolveFrom} from "./utility/resolveFrom";

class ProjectPath {
    ekc = path.dirname(resolveFrom(__dirname, "@ekx/ekc/package.json"));
    EKX_ROOT = path.dirname(resolveFrom(__dirname, "@ekx/ekx/package.json"));
    CURRENT_PROJECT_DIR = process.cwd();
    OUTPUT = path.join(process.cwd(), "build");

    cli = path.resolve(__dirname, '..');
    templates = path.resolve(__dirname, '../templates');

    dump() {
        for (const [k, v] of Object.entries(this)) {
            console.info(`${k}: ${v}`);
        }
    }
}

type RegisteredProject = any;

export type LegacySourceKind = "cpp" | "js" | "java" | "aidl" | "assets";
export type LegacySources = string[];

interface ModuleDef {
    name?: string;
    cpp?: LegacySources;
    assets?: string[];
    android?: {
        cpp?: LegacySources;
        java?: LegacySources;
        aidl?: LegacySources;
        xmlStrings?: { [name: string]: string };
    };
    macos?: {
        cpp?: LegacySources;
    };
    ios?: {
        cpp?: LegacySources;
        cpp_flags?: {
            files?: string[];
            flags?: string;
        };
        xcode?: {
            capabilities?: string[];
            frameworks?: string[];
            pods?: string[];
            plist?: any[];
        };
    };
    web?: {
        cpp?: LegacySources;
    };
    windows?: {
        cpp?: LegacySources;
    };
    linux?: {
        cpp?: LegacySources;
    };
}

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

    build_steps: (() => void|Promise<any>)[] = [];
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

    build = {
        android: {
            dependencies: [],
            add_manifest: [],
            add_manifest_application: [],
            source_dirs: [],
            xmlStrings: {}
        }
    };

    android: {
        // android application package name
        application_id?: string,
        // just package for java code
        package_id?: string,
        googleServicesConfigDir?: string,
        signingConfigPath?: string,
        // path to service account api key json file, used for fastlane automation
        serviceAccountKey?: string
    };

    ios: {
        application_id?: string,
        googleServicesConfigDir?: string,
        // path to JSON with app-store credentials, used for fastlane automation
        appStoreCredentials?: string
    } = {};

    web: {
        firebaseToken?: string,
        applications?: { platform: string, url: string, id?: string }[]
    } = {};

    html = {};

    addModule(def: ModuleDef) {
        this.modules.push(def);
        if (def && def.name) {
            console.info("Module:", def.name);
        }
    }

    includeProject(projectPath: string) {
        if (this.projects[projectPath]) {
            return;
        }

        const project_js = path.join(projectPath, "ek.js");
        let projectConfigurator = null;
        try {
            projectConfigurator = require(project_js);
        } catch {
            console.error("Project not included", project_js);
        }

        if (projectConfigurator) {
            this.projects[projectPath] = projectConfigurator(this);
        }
    }

    importModule(moduleId: string, fromDir?: string) {
        fromDir = fromDir ?? process.cwd();
        const projectPath = resolveFrom(fromDir, moduleId + "/ek.js");
        if (projectPath == null) {
            console.warn("ek.js module not found")
        }
        this.includeProject(path.dirname(projectPath));
    }

    async runBuildSteps() {
        for (const step of this.build_steps) {
            const res = step();
            if(res instanceof Promise) {
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
    }
}