import * as path from "path";

class ProjectPath {
    EKX_ROOT = process.env.EKX_ROOT ?? path.resolve(__dirname, '../..');
    CURRENT_PROJECT_DIR = process.cwd();
    OUTPUT = path.join(process.cwd(), "build");
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
        openProject?: boolean
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
        // used for automation
        appleId?: string
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

    runBuildSteps() {
        for (const step of this.build_steps) {
            step();
        }
    }
}