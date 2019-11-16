#!/usr/bin/env node

project = {};
project.path = {};

console.debug("Arguments", process.argv);
project.current_target = process.argv[2];

const path = require('path');

project.path.EKX_ROOT = process.env.EKX_ROOT;
project.path.CURRENT_PROJECT_DIR = process.cwd();
project.path.OUTPUT = project.path.CURRENT_PROJECT_DIR + "/build";

EK = {
    require: function (id) {
        try {
            return require(id);
        } catch {
        }
        return require(`${project.path.EKX_ROOT}/cli/node_modules/${id}`);
    }
};

// if(process.env.NODE_PATH) {
//     process.env.NODE_PATH += ":" + `${project.path.EKX_ROOT}/cli/node_modules`;
// }
// else {
//     process.env.NODE_PATH = `${project.path.EKX_ROOT}/cli/node_modules`;
// }
// console.log(process.env.NODE_PATH);

project.build_steps = [];
project.projects = {};
project.include_project = function (project_dir) {
    project_js = path.join(project_dir, "ek.js");
    let ProjectFile = null;
    try {
        ProjectFile = require(project_js);
    } catch {
        console.warn("Project not included", project_js);
    }

    if (ProjectFile) {
        this.projects[project_dir] = new ProjectFile(project);
    }
};

for (const [k, v] of Object.entries(project.path)) {
    console.info(`${k}: ${v}`);
}

project.include_project(project.path.EKX_ROOT + "/ek");
project.include_project(process.cwd());

for (const build_step of project.build_steps) {
    build_step();
}

