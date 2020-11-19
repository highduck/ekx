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

    build_steps: (() => void)[] = [];
    projects: { [name: string]: RegisteredProject } = {};
    modules: ModuleDef[] = [];

    assets: {
        output: string
    };
    market_asset: string;

    build = {
        android: {
            dependencies: [],
            add_manifest: [],
            add_manifest_application: [],
            source_dirs: [],
        }
    };

    ios = {};
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