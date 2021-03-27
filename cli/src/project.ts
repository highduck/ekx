import * as path from "path";
import {VERSION_INDEX_MAJOR, VERSION_INDEX_MINOR, VERSION_INDEX_PATCH} from "./version";
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

interface ModuleDef {
    name?: string;
    cpp?: string[];
    assets?: string[];
    android?: {
        cpp?: string[];
        java?: string[];
        aidl?: string[];
        xmlStrings?: { [name: string]: string };
    };
    macos?: {
        cpp?: string[];
    };
    ios?: {
        cpp?: string[];
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
        cpp?: string[];
    };
    windows?: {
        cpp?: string[];
    };
    linux?: {
        cpp?: string[];
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
        deployBeta?: boolean
        increaseVersion?: number
    } = {};

    name: string;
    version_name: string;
    version_code: string;
    build_steps: (() => void)[] = [];
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
        firebaseToken?: string
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

    runBuildSteps() {
        for (const step of this.build_steps) {
            step();
        }
    }

    constructor() {
        if(this.args.indexOf("clean") >= 0) {
            this.options.clean = true;
        }

        if(this.args.indexOf("-o") >= 0) {
            this.options.openProject = true;
        }
        else if(this.args.indexOf("do-not-open") < 0) {
            this.options.openProject = false;
        }

        if(this.args.indexOf("patch") >= 0) {
            this.options.increaseVersion = VERSION_INDEX_PATCH;
        }
        else if(this.args.indexOf("minor") >= 0) {
            this.options.increaseVersion = VERSION_INDEX_MINOR;
        }
        else if(this.args.indexOf("major") >= 0) {
            this.options.increaseVersion = VERSION_INDEX_MAJOR;
        }

        this.options.deployBeta = this.args.indexOf("beta") >= 0;
    }
}