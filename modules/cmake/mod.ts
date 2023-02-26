import * as fs from "fs";
import * as path from "path";
import * as os from "os";
import {ensureDirSync, getModuleDir, rm, run} from "../utils/utils.js";

const __dirname = getModuleDir(import.meta);
export interface BuildMatrix {
    os?: string | string[];
    buildType?: string | string[];
    parallel?: boolean;
}

type OSName = "windows" | "linux" | "macos" | "ios" | "android" | "web";
const getOsName = (val: string = os.platform()): OSName => {
    switch (val) {
        case "darwin":
            return "macos";
        case "linux":
            return "linux";
        case "win32":
            return "windows";
    }
    return val as OSName;
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

    // target build operating system
    os?: OSName | "ios" | "android" | "web";

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

export interface ExecuteOptions {
    verbose?: boolean;
    workingDir?: string;
    env?: { [option: string]: string };
}

export async function executeAsync(bin: string, args: string[], options: ExecuteOptions): Promise<number> {
    const result = await run({
        cmd: [bin].concat(args),
        env: options.env ?? process.env,
        cwd: options.workingDir,
        stdio: options.verbose ? "inherit" : "ignore"
    });
    if (!result.success) {
        throw new Error(`Run failed with exit code: ${result.code}`);
    }
    return 0;
}

function getBuildDirName(platform: string, buildType: string): string {
    return `${platform}-${buildType.toLowerCase()}`;
}

function getEmscriptenSDKPath(): string {
    const v = process.env.EMSDK;
    if (v) {
        return v;
    }
    return path.join(process.env.HOME ?? "~", "dev/emsdk");
}

function getNDKPath(): string {
    const names = ["ANDROID_NDK_LATEST_HOME", "ANDROID_NDK_HOME", "ANDROID_NDK_ROOT"];
    for (const env of names) {
        const ndkPath = process.env[env];
        if (ndkPath && fs.existsSync(ndkPath)) {
            return ndkPath;
        }
    }
    return path.resolve(process.env.HOME ?? "~", "Library/Android/sdk/ndk/23.1.7779620");
}

export function resolveOptions(options?: BuildOptions): Required<BuildOptions> {
    const opts = options ?? {};
    opts.env = opts.env ?? {};
    opts.debug = opts.debug ?? false;
    opts.buildType = opts.buildType ?? (opts.debug ? "Debug" : "Release");
    opts.os = getOsName(opts.os);
    opts.workingDir = opts.workingDir ?? process.cwd();
    opts.cmakePath = opts.cmakePath ?? ".";
    opts.buildsFolder = opts.buildsFolder ?? path.resolve(opts.workingDir, "build");
    opts.buildDir = opts.buildDir ?? path.resolve(opts.buildsFolder, getBuildDirName(opts.os!, opts.buildType));
    opts.ccache = opts.ccache ?? true;
    opts.ninja = opts.ninja ?? true;
    opts.definitions = opts.definitions ?? {};
    opts.clean = opts.clean ?? true;
    opts.configure = opts.configure ?? true;
    opts.build = opts.build ?? true;
    opts.test = opts.test ?? false;
    opts.target = getOptionalMany(opts.target, []);

    opts.cc = opts.cc ?? process.env.CC;
    opts.cxx = opts.cxx ?? process.env.CXX;

    opts.compiler = opts.compiler ?? "clang";
    if (opts.compiler === "clang") {
        if (!opts.cc) opts.cc = "clang";
        if (!opts.cxx) opts.cxx = "clang++";
    }

    opts.ccache = opts.ccache ?? !!(process.env.USE_CCACHE as string | 0);
    if (opts.ccache && opts.os !== getOsName()) {
        if (opts.os !== "web") {
            opts.ccache = false;
        }
    }

    if (opts.ccache) {
        opts.cc = "ccache";
        opts.cxx = "ccache";
    }

    if (opts.os !== getOsName()) {
        switch (opts.os) {
            case "web": {
                if (!opts.toolchain) {
                    const sdk = getEmscriptenSDKPath();
                    opts.toolchain = path.resolve(sdk, "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake");
                }
                const nodePath = process.env.NODE;
                if (!process.env.EM_NODE_JS && nodePath) {
                    process.env.EM_NODE_JS = nodePath;
                }
                //options.cc = undefined;
                //options.cxx = undefined;
            }
                break;
            case "android":
                if (!opts.toolchain) {
                    opts.toolchain = path.join(getNDKPath(), "build/cmake/android.toolchain.cmake");
                }
                opts.definitions.ANDROID_ABI = "x86_64";
                opts.cc = undefined;
                opts.cxx = undefined;
                break;
            case "ios":
                if (!opts.toolchain) {
                    opts.toolchain = path.resolve(__dirname, "ios.cmake");
                }
                opts.definitions.PLATFORM = "SIMULATOR64";
                break;
            case "windows":
                if (os.platform() === "darwin") {
                    if (!opts.toolchain) {
                        opts.toolchain = path.resolve(__dirname, "mingw-w64-x86_64.cmake");
                        console.info(opts.toolchain);
                    }
                    opts.cc = undefined;
                    opts.cxx = undefined;
                } else {
                    throw new Error("not supported");
                }
                break;
        }
    }

    return opts as Required<BuildOptions>;
}

export async function clean(options: Required<BuildOptions>): Promise<void> {
    if (options.buildDir) {
        console.info("Clean build directory:", options.buildDir);
        await rm(options.buildDir);
    }
}

export async function configure(options: Required<BuildOptions>): Promise<void> {
    console.info("Configure");
    if (options.cmakePath == null || options.buildDir == null) {
        throw new Error("Bad arguments: cmakePath or buildDir");
    }
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
    if (options.definitions) {
        for (const key of Object.keys(options.definitions)) {
            args.push(`-D${key}=${options.definitions[key]}`);
        }
    }

    const executionOptions = {
        verbose: true,
        workingDir: options.workingDir,
        env: Object.assign({}, process.env, options.env)
    };
    await executeAsync("cmake", args, executionOptions);
}

export async function build_(options: Required<BuildOptions>): Promise<void> {
    console.info("Build");
    const executionOptions = {
        verbose: true,
        workingDir: options.workingDir,
        env: Object.assign({}, process.env, options.env)
    };
    const buildArgs = ["--build", options.buildDir!];
    if (!options.ninja) {
        const jobs = 1 + os.cpus().length;
        buildArgs.push("--parallel", "" + jobs);
    }
    if (options.target!.length > 0) {
        buildArgs.push("--target");
        for (const target of options.target!) {
            buildArgs.push(target);
        }
    }
    await executeAsync("cmake", buildArgs, executionOptions);
}

export async function test_(options: Required<BuildOptions>): Promise<void> {
    console.info("Test");
    const executionOptions = {
        verbose: true,
        workingDir: path.resolve(options.workingDir!, options.buildDir!),
        env: Object.assign({}, process.env, options.env, {CTEST_OUTPUT_ON_FAILURE: "TRUE"})
    };

    if (options.ninja) {
        await executeAsync("ninja", ["test"], executionOptions);
    } else {
        await executeAsync("make", ["test"], executionOptions);
    }
}

export async function build(options?: BuildOptions & BuildMatrix): Promise<BuildResult> {
    const opts = resolveOptions(options);
    if (opts.clean) {
        await clean(opts);
    }
    ensureDirSync(opts.buildDir);
    if (opts.configure) {
        await configure(opts);
    }
    if (opts.build) {
        await build_(opts);
    }
    if (opts.test) {
        await test_(opts);
    }
    return {
        buildDir: path.resolve(opts.workingDir!, opts.buildDir!)
    };
}

export async function buildMatrix(options?: BuildOptions & BuildMatrix): Promise<void> {
    const buildTypes = getOptionalMany(options?.buildType, ["Debug", "Release"]);
    const osList = getOptionalMany(options?.os, [getOsName()]);
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
        } else {
            return v;
        }
    }
    return defaultValue;
}
