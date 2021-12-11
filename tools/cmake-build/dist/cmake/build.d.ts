export interface BuildMatrix {
    os?: string | string[];
    buildType?: string | string[];
    parallel?: boolean;
}
export interface BuildOptions {
    debug?: boolean;
    buildType?: string;
    buildsFolder?: string;
    buildDir?: string;
    os?: string;
    compiler?: "clang";
    ccache?: boolean;
    cc?: string;
    cxx?: string;
    toolchain?: string;
    ninja?: boolean;
    definitions?: {
        [option: string]: string;
    };
    args?: string[];
    cmakePath?: string;
    workingDir?: string;
    env?: {
        [option: string]: string;
    };
    target?: string | string[];
    clean?: boolean;
    configure?: boolean;
    build?: boolean;
    test?: boolean;
}
export interface BuildResult {
    buildDir: string;
}
export declare function resolveOptions(options?: BuildOptions): BuildOptions;
export declare function clean(options: BuildOptions): Promise<void>;
export declare function configure(options: BuildOptions): Promise<void>;
export declare function build_(options: BuildOptions): Promise<void>;
export declare function test_(options: BuildOptions): Promise<void>;
export declare function build(options?: BuildOptions & BuildMatrix): Promise<BuildResult>;
export declare function buildMatrix(options?: BuildOptions & BuildMatrix): Promise<void>;
