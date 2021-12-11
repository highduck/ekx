"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.buildMatrix = exports.build = exports.test_ = exports.build_ = exports.configure = exports.clean = exports.resolveOptions = void 0;
const child_process_1 = require("child_process");
const path = require("path");
const fs = require("fs");
const OS = require("os");
function executeAsync(bin, args, options) {
    return new Promise((resolve, reject) => {
        const child = (0, child_process_1.spawn)(bin, args, {
            stdio: options.verbose ? "inherit" : "ignore",
            cwd: options.workingDir,
            env: options.env ?? process.env
        });
        child.on('close', (code) => {
            if (code === 0) {
                resolve(code);
            }
            else {
                reject('exit code: ' + code);
            }
        });
        child.on("error", (err) => {
            reject(err);
        });
    });
}
function getBuildDirName(platform, buildType) {
    return platform + "-" + buildType.toLowerCase();
}
function getEmscriptenSDKPath() {
    if (process.env.EMSDK) {
        return process.env.EMSDK;
    }
    return path.join(process.env.HOME, "dev/emsdk");
}
function resolveOptions(options) {
    options = options ?? {};
    options.env = options.env ?? {};
    options.debug = options.debug ?? false;
    options.buildType = options.buildType ?? (options.debug ? "Debug" : "Release");
    options.os = options.os ?? process.platform;
    options.workingDir = options.workingDir ?? process.cwd();
    options.cmakePath = options.cmakePath ?? ".";
    options.buildsFolder = options.buildsFolder ?? path.resolve(options.workingDir, "build");
    options.buildDir = options.buildDir ?? path.resolve(options.buildsFolder, getBuildDirName(options.os, options.buildType));
    options.ccache = options.ccache ?? true;
    options.ninja = options.ninja ?? true;
    options.definitions = options.definitions ?? {};
    options.clean = options.clean ?? true;
    options.configure = options.configure ?? true;
    options.build = options.build ?? true;
    options.test = options.test ?? false;
    options.target = getOptionalMany(options.target, []);
    options.cc = options.cc ?? process.env.CC;
    options.cxx = options.cxx ?? process.env.CXX;
    options.compiler = options.compiler ?? "clang";
    if (options.compiler === "clang") {
        if (!options.cc)
            options.cc = "clang";
        if (!options.cxx)
            options.cxx = "clang++";
    }
    options.ccache = options.ccache ?? !!process.env.USE_CCACHE;
    if (options.ccache && options.os !== process.platform) {
        if (options.os !== "web") {
            options.ccache = false;
        }
    }
    if (options.ccache) {
        options.cc = "ccache";
        options.cxx = "ccache";
    }
    switch (options.os) {
        case "web":
            if (!options.toolchain) {
                const sdk = getEmscriptenSDKPath();
                options.toolchain = path.resolve(sdk, "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake");
            }
            if (!process.env.EM_NODE_JS && process.env.NODE) {
                options.env.EM_NODE_JS = process.env.NODE;
            }
            //options.cc = undefined;
            //options.cxx = undefined;
            break;
        case "android":
            if (!options.toolchain) {
                options.toolchain = path.resolve(process.env.HOME, "Library/Android/sdk/ndk/23.0.7599858/build/cmake/android.toolchain.cmake");
            }
            options.definitions.ANDROID_ABI = "x86_64";
            options.cc = undefined;
            options.cxx = undefined;
            break;
        case "ios":
            if (!options.toolchain) {
                options.toolchain = path.resolve(__dirname, "../../cmake/ios.cmake");
            }
            options.definitions.PLATFORM = "SIMULATOR64";
            break;
        case "windows":
            if (process.platform === "darwin") {
                if (!options.toolchain) {
                    options.toolchain = path.resolve(__dirname, "../../cmake/mingw-w64-x86_64.cmake");
                    console.info(options.toolchain);
                }
                options.cc = undefined;
                options.cxx = undefined;
            }
            else {
                throw new Error("not supported");
            }
            break;
    }
    return options;
}
exports.resolveOptions = resolveOptions;
async function clean(options) {
    try {
        console.info("cleanup build directory:", options.buildDir);
        await fs.promises.rm(options.buildDir, { recursive: true });
    }
    catch {
    }
}
exports.clean = clean;
async function configure(options) {
    console.info("Configure");
    const args = [
        "-S", options.cmakePath,
        "-B", options.buildDir
    ];
    if (options.ninja) {
        args.push("-G", "Ninja");
    }
    if (options.cc) {
        args.push(`-DCMAKE_C_COMPILER_LAUNCHER=${options.cc}`);
    }
    if (options.cxx) {
        args.push(`-DCMAKE_CXX_COMPILER_LAUNCHER=${options.cxx}`);
    }
    if (options.toolchain) {
        args.push(`-DCMAKE_TOOLCHAIN_FILE=${options.toolchain}`);
    }
    args.push(`-DCMAKE_BUILD_TYPE=${options.buildType}`);
    for (const key of Object.keys(options.definitions)) {
        args.push(`-D${key}=${options.definitions[key]}`);
    }
    const executionOptions = {
        verbose: true,
        workingDir: options.workingDir,
        env: Object.assign({}, process.env, options.env)
    };
    await executeAsync("cmake", args, executionOptions);
}
exports.configure = configure;
async function build_(options) {
    console.info("Build");
    const executionOptions = {
        verbose: true,
        workingDir: options.workingDir,
        env: Object.assign({}, process.env, options.env)
    };
    const buildArgs = ["--build", options.buildDir];
    if (!options.ninja) {
        const jobs = 1 + OS.cpus().length;
        buildArgs.push("--parallel", "" + jobs);
    }
    if (options.target.length > 0) {
        buildArgs.push("--target");
        for (const target of options.target) {
            buildArgs.push(target);
        }
    }
    await executeAsync("cmake", buildArgs, executionOptions);
}
exports.build_ = build_;
async function test_(options) {
    console.info("Test");
    const executionOptions = {
        verbose: true,
        workingDir: path.resolve(options.workingDir, options.buildDir),
        env: Object.assign({}, process.env, options.env, { CTEST_OUTPUT_ON_FAILURE: "TRUE" })
    };
    if (options.ninja) {
        await executeAsync("ninja", ["test"], executionOptions);
    }
    else {
        await executeAsync("make", ["test"], executionOptions);
    }
}
exports.test_ = test_;
async function build(options) {
    options = resolveOptions(options);
    if (options.clean) {
        await clean(options);
    }
    if (options.configure) {
        await configure(options);
    }
    if (options.build) {
        await build_(options);
    }
    if (options.test) {
        await test_(options);
    }
    return {
        buildDir: path.resolve(options.workingDir, options.buildDir)
    };
}
exports.build = build;
async function buildMatrix(options) {
    const buildTypes = getOptionalMany(options?.buildType, ["Debug", "Release"]);
    const osList = getOptionalMany(options?.os, [process.platform]);
    if (options?.parallel) {
        const tasks = [];
        for (const os of osList) {
            for (const buildType of buildTypes) {
                tasks.push(build(Object.assign({}, options, {
                    os,
                    buildType
                })));
            }
        }
        await Promise.all(tasks);
    }
    else {
        for (const os of osList) {
            for (const buildType of buildTypes) {
                await build(Object.assign({}, options, {
                    os,
                    buildType
                }));
            }
        }
    }
}
exports.buildMatrix = buildMatrix;
function getOptionalMany(v, defaultValue) {
    if (v) {
        if (typeof v === "string") {
            return [v];
        }
        else if (v instanceof Array) {
            return v;
        }
    }
    return defaultValue;
}
