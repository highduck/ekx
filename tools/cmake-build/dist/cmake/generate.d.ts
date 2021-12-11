export interface CMakeGenerateTarget {
    type: "library" | "executable";
    libraryType: "shared" | "static" | "interface";
    name: string;
    sources: string[];
    includeDirectories: string[];
    linkLibraries: string[];
    linkOptions: string[];
    compileOptions: string[];
    compileDefinitions: string[];
}
export interface CMakeGenerateProject {
    cmakeVersion: string;
    project: string;
    targets: CMakeGenerateTarget[];
    compileOptions: string[];
    compileDefinitions: string[];
}
export declare function cmakeLists(project: CMakeGenerateProject): string;
