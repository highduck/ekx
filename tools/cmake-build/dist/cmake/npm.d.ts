export declare function resolveFrom(fromDirectory: string, moduleId: string): string;
export declare function convertPackageName(name: string): string;
export declare function dependencyBlock(name: string, dep: string, rel: string, dir: string): string;
export declare function collectDependencies(dependencies: {
    [key: string]: string;
} | undefined, output: string[]): void;
export interface Pkg {
    name: string;
    version: string;
    dependencies?: {
        [key: string]: string;
    };
    devDependencies?: {
        [key: string]: string;
    };
    peerDependencies?: {
        [key: string]: string;
    };
}
export declare function readPkg(dir: string): null | Pkg;
