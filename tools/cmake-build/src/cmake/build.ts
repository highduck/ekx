import {spawn} from "child_process";
import * as path from "path";
import * as fs from "fs";
import * as OS from "os";

export interface BuildMatrix {
    os?: string | string[];
    buildType?: string | string[];
    parallel?: boolean;
}

export interface BuildOptions {
    // if build-type is not explicitly set, this value used to detect if you need Debug or Release build
    // false by default
    debug?: boolean;
    buildType?: string;
    // `build` by default
    buildsFolder?: string;
    // explicit build dir, by default it will generate as `${buildsFolder}/platform-type`
    buildDir?: string;

    // target build operation system
    os?: string;

    compiler?: "clang";
    // by default detect from USE_CCACHE env variable (value should be non-empty, non-zero)
    ccache?: boolean;
    cc?: string;
    cxx?: string;
    toolchain?: string;
    // true by default
    ninja?: boolean;
    definitions?: { [option: string]: string };
    args?: string[];

    // path to directory with project's `CMakeLists.txt` (search-path depends on workingDir)
    // "." by default
    cmakePath?: string;

    workingDir?: string;

    env?: { [option: string]: string };

    // build targets, if empty - build ALL targets
    // empty by default
    target?: string | string[];

    // tasks
    // true by default
    clean?: boolean;
    // true by default
    configure?: boolean;
    // true by default
    build?: boolean;
    // false by default
    test?: boolean;
}

export interface BuildResult {
    buildDir: string;
}

interface ExecuteOptions {
    verbose?: boolean;
    workingDir?: string;
    env?: { [option: string]: string };
}

function executeAsync(bin: string, args: string[], options: ExecuteOptions): Promise<number> {
    return new Promise((resolve, reject) => {
        const child = spawn(bin, args, {
            stdio: options.verbose ? "inherit" : "ignore",
            cwd: options.workingDir,
            env: options.env ?? process.env
        });
        child.on('close', (code) => {
            if (code === 0) {
                resolve(code);
            } else {
                reject('exit code: ' + code);
            }
        });
        child.on("error", (err) => {
            reject(err);
        });
    });
}

function getBuildDirName(platform: string, buildType: string): string {
    return platform + "-" + buildType.toLowerCase();
}

function getEmscriptenSDKPath(): string {
    if (process.env.EMSDK) {
        return process.env.EMSDK;
    }
    return path.join(process.env.HOME, "dev/emsdk");
}

export function resolveOptions(options?: BuildOptions): BuildOptions {
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
        if (!options.cc) options.cc = "clang";
        if (!options.cxx) options.cxx = "clang++";
    }

    options.ccache = options.ccache ?? !!(process.env.USE_CCACHE as any | 0);
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
            } else {
                throw new Error("not supported");
            }
            break;
    }

    return options;
}

export async function clean(options: BuildOptions): Promise<void> {
    try {
        console.info("cleanup build directory:", options.buildDir);
        await fs.promises.rm(options.buildDir, {recursive: true});
    } catch {
    }
}

export async function configure(options: BuildOptions): Promise<void> {
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

export async function build_(options: BuildOptions): Promise<void> {
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

export async function test_(options: BuildOptions): Promise<void> {
    console.info("Test");
    const executionOptions = {
        verbose: true,
        workingDir: path.resolve(options.workingDir, options.buildDir),
        env: Object.assign({}, process.env, options.env, {CTEST_OUTPUT_ON_FAILURE: "TRUE"})
    };

    if (options.ninja) {
        await executeAsync("ninja", ["test"], executionOptions);
    } else {
        await executeAsync("make", ["test"], executionOptions);
    }
}

export async function build(options?: BuildOptions & BuildMatrix): Promise<BuildResult> {
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

export async function buildMatrix(options?: BuildOptions & BuildMatrix): Promise<void> {
    const buildTypes = getOptionalMany(options?.buildType, ["Debug", "Release"]);
    const osList = getOptionalMany(options?.os, [process.platform]);
    if (options?.parallel) {
        const tasks: Promise<BuildResult>[] = [];
        for (const os of osList) {
            for (const buildType of buildTypes) {
                tasks.push(
                    build(Object.assign({}, options, {
                        os,
                        buildType
                    }))
                );
            }
        }
        await Promise.all(tasks);
    } else {
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

function getOptionalMany(v: string | string[] | null | undefined, defaultValue: string[]): string[] {
    if (v) {
        if (typeof v === "string") {
            return [v];
        } else if (v as any instanceof Array) {
            return v;
        }
    }
    return defaultValue;
}