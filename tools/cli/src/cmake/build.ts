import {executeAsync} from "../utils";

export interface CMakeExportOptions {
    debug?: boolean;
    buildType?: string;
    ccache?: boolean;
    ninja?: boolean;
    definitions?: { [option: string]: string };
    workingDir?: string;
    toolchain?: string;
}

export async function cmake(dir?: string, options?: CMakeExportOptions) {
    options = options ?? {};
    options.debug = options.debug ?? false;
    options.buildType = options.buildType ?? (options.debug ? "Debug" : "Release");
    options.ccache = options.ccache ?? true;
    options.ninja = options.ninja ?? true;
    options.definitions = options.definitions ?? {};
    options.workingDir = options.workingDir ?? process.cwd();
    const buildDir = dir ?? `cmake-build-${options.buildType.toLowerCase()}`;
    const args = [
        "-S", ".",
        "-B", buildDir
    ];

    if (options.ninja) {
        args.push("-G", "Ninja");
    }
    if (options.ccache) {
        args.push("-DCMAKE_C_COMPILER_LAUNCHER=ccache", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache");
    }
    if (options.toolchain) {
        args.push(`-DCMAKE_TOOLCHAIN_FILE=${options.toolchain}`);
    }
    args.push(`-DCMAKE_BUILD_TYPE=${options.buildType}`);
    for (const key of Object.keys(options.definitions)) {
        args.push(`-D${key}=${options.definitions[key]}`);
    }

    await executeAsync("cmake", args, {workingDir: options.workingDir});
    await executeAsync("cmake", ["--build", buildDir], {workingDir: options.workingDir});
}