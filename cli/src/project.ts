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

export class Project {
    readonly path = new ProjectPath();
    current_target = process.argv[2];

    build_steps: (() => void)[] = [];
    projects: { [name: string]: RegisteredProject } = {};
    modules = [];

    assets: {
        output: string
    };
    market_asset: string;

    include_project(project_dir: string) {
        const project_js = path.join(project_dir, "ek.js");
        let projectConfigurator = null;
        try {
            projectConfigurator = require(project_js);
        } catch {
            console.error("Project not included", project_js);
        }

        if (projectConfigurator) {
            this.projects[project_dir] = projectConfigurator(this);
        }
    }

    runBuildSteps() {
        for (const step of this.build_steps) {
            step();
        }
    }
}