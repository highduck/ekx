#!/usr/bin/env node

const {spawnSync} = require('child_process');

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
    },
    execute: function (cmd, args) {
        console.debug(">> " + [cmd].concat(args).join(" "));
        console.debug("cwd", process.cwd());
        const child = spawnSync(cmd, args, {
                stdio: 'pipe',
                encoding: 'utf-8',
                cwd: process.cwd()
            }
        );
        console.log("stderr", child.stderr.toString());
        console.log("stdout", child.stdout.toString());
        console.log("exit code", child.status);
        if (child.error) {
            console.error(child.error);
        }

        return child.status;
    },
    optimize_png: function (input, output = undefined) {
        const pngquant = require('pngquant-bin');
        if (!output) output = input;
        const result = spawnSync(pngquant, [
            "--strip",
            "--force",
            "-o", output,
            input
        ]);
        if (result.status === 0) {
            console.log('Image minified! ' + input);
        } else {
            console.warn(result.stderr.toString());
            console.warn(result.status);
        }
    },
    optimize_png_glob: function (input_pattern) {
        const glob = require('glob');
        const files = glob.sync(input_pattern);
        for (const file of files) {
            EK.optimize_png(file, file);
        }
    },
    with_path: function (path, cb) {
        const p = process.cwd();
        process.chdir(path);
        cb();
        process.chdir(p);
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

project.include_project(path.join(project.path.EKX_ROOT, "ek"));
project.include_project(path.join(project.path.EKX_ROOT, "core"));
project.include_project(path.join(project.path.EKX_ROOT, "scenex"));
project.include_project(path.join(project.path.EKX_ROOT, "extensions/audiomini"));
project.include_project(process.cwd());

for (const build_step of project.build_steps) {
    build_step();
}

