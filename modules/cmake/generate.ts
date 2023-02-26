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
    sourceFileCompileFlags?: {files:string[], flags:string}[];
}

export interface CMakeGenerateProject {
    cmakeVersion: string; // pass 3.19 as default
    project: string;
    targets: CMakeGenerateTarget[];

    compileOptions: string[];
    compileDefinitions: string[];
}

export const cmakeLists = (project: CMakeGenerateProject): string => {
    const lines: string[] = [];
    if (project.cmakeVersion) {
        lines.push(`cmake_minimum_required(VERSION ${project.cmakeVersion})`);
    }
    if (project.project) {
        lines.push(`project(${project.project})`);
    }
    lines.push("\n");
    for (const target of project.targets) {
        if (target.type === "library") {
            const libraryType = target.libraryType.toUpperCase();
            lines.push(`add_library(${target.name} ${libraryType}`);
            for (const source of target.sources) {
                lines.push("\t\t" + source);
            }
            lines.push(`)`);
        } else {
            lines.push(`add_executable(${target.name}`);
            for (const source of target.sources) {
                lines.push("\t\t" + source);
            }
            lines.push(`)`);
        }
        if (target.includeDirectories && target.includeDirectories.length > 0) {
            lines.push(`target_include_directories(${target.name}`);
            for (const includes of target.includeDirectories) {
                // INTERFACE|PUBLIC|PRIVATE
                lines.push("\t\tPUBLIC " + includes);
            }
            lines.push(`)`);
        }

        lines.push(`set_target_properties(${target.name} PROPERTIES
            C_STANDARD 11
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
        )`);

        if (target.linkLibraries.length > 0) {
            lines.push(`target_link_libraries(${target.name}`);
            for (const lib of target.linkLibraries) {
                lines.push("\t\tPRIVATE " + lib);
            }
            lines.push(`)`);
        }

        if (target.linkOptions.length > 0) {
            lines.push(`target_link_options(${target.name}`);
            for (const linkOption of target.linkOptions) {
                lines.push("\t\tPUBLIC " + linkOption);
            }
            lines.push(`)`);
        }

        if (target.compileDefinitions.length > 0) {
            lines.push(`target_compile_definitions(${target.name}`);
            for (const compileDefinition of target.compileDefinitions) {
                lines.push("\t\tPUBLIC " + compileDefinition);
            }
            lines.push(`)`);
        }

        if(target.sourceFileCompileFlags && target.sourceFileCompileFlags.length > 0) {
            for(const srcCFlags of target.sourceFileCompileFlags) {
                for(const src of srcCFlags.files) {
                    lines.push(`\t\tset_source_files_properties("${src}" PROPERTIES COMPILE_FLAGS "${srcCFlags.flags}")`);
                }
            }
        }

        if (target.compileOptions.length > 0) {
            lines.push(`target_compile_options(${target.name}`);
            for (const compileOption of target.compileOptions) {
                lines.push("\t\tPUBLIC " + compileOption);
            }
            lines.push(`)`);
        }
    }

    if (project.compileOptions.length > 0) {
        lines.push(`add_compile_options(${project.compileOptions.join(" ")})`);
    }

    if (project.compileDefinitions.length > 0) {
        lines.push(`add_compile_definitions(${project.compileDefinitions.join(" ")})`);
    }

    return lines.join("\n");
};