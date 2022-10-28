import {writeTextFileSync} from "../utils/utils.js";

export type DependencyType = "classpath" | "implementation" | "compile";

export class GradlePrinter {
    _indent = 0;
    _buf = "";

    indent() {
        ++this._indent;
    }

    unindent() {
        --this._indent;
    }

    writeln(line: string) {
        for (let i = 0; i < this._indent; ++i) {
            this._buf += "\t";
        }
        this._buf += line;
        this._buf += "\n";
    }

    block(header: string, fn?: () => void) {
        if (fn) {
            this.writeln(header + " {");
            this.indent();
            fn();
            this.unindent();
            this.writeln("}");
        } else {
            this.writeln(header + "()");
        }
    }

    printRepository(rep: Repository) {
        this.block(rep.name, rep.url ? () => {
            this.writeln(`url "${rep.url}"`);
        } : undefined);
    }

    printDependency(dep: Dependency | string) {
        if (typeof dep === "string") {
            this.writeln(dep);
        } else {
            let str = dep.name;
            if (dep.version) {
                str += ":";
                str += dep.version;
            }
            this.writeln(`${dep.type ?? "implementation"} '${str}'`);
        }
    }

    printRepositorySection(repositories?: Repository[]) {
        if (repositories) {
            this.block("repositories", () => {
                for (const rep of repositories) {
                    this.printRepository(rep);
                }
            });
        }
    }

    printDependencySection(dependencies?: (Dependency | string)[]) {
        if (dependencies) {
            this.block("dependencies", () => {
                for (const dep of dependencies) {
                    this.printDependency(dep);
                }
            });
        }
    }

    printPlugins(plugins?: string[]) {
        if (plugins) {
            this.block("plugins", () => {
                for (const plugin of plugins) {
                    this.writeln(`id("${plugin}")`);
                }
            });
        }
    }

    printCmake(cmake?: Cmake) {
        if (cmake) {
            this.block("cmake", () => {
                this.printString(cmake, "path");
                this.printString(cmake, "version");
                this.printString(cmake, "cppFlags");
            });
        }
    }

    printExternalNativeBuild(externalNativeBuild?: ExternalNativeBuild) {
        if (externalNativeBuild) {
            this.block("externalNativeBuild", () => {
                this.printCmake(externalNativeBuild.cmake);
            });
        }
    }

    printCompileOptions(javaVersion?: string) {
        if (javaVersion) {
            this.block("compileOptions", () => {
                const javaVersionConst = "JavaVersion.VERSION_" + javaVersion.replace(".", "_");
                this.writeln(`sourceCompatibility ${javaVersionConst}`);
                this.writeln(`targetCompatibility ${javaVersionConst}`);
            });
        }
    }

    static toLiteral(v: any) {
        switch (typeof v) {
            case "string":
                return `'${v}'`;
            case "number":
                return v.toString();
        }
        return "" + v;
    }

    printString(data: any, name: string) {
        if (data && data[name] !== undefined) {
            const val = data[name];
            if (val instanceof Array) {
                if (val.length > 0) {
                    const args = val.map(GradlePrinter.toLiteral).join(", ");
                    this.writeln(`${name} ${args}`);
                }
            } else {
                this.writeln(`${name} ${GradlePrinter.toLiteral(val)}`);
            }
        }
    }

    printExpr(data: any, name: string) {
        if (data && data[name] !== undefined) {
            const val = data[name];
            if (val instanceof Array) {
                this.writeln(`${name} ${val.join(", ")}`);
            } else {
                this.writeln(`${name} ${val}`);
            }
        }
    }

    printNumber(data: any, name: string) {
        if (data && data[name] !== undefined) {
            this.writeln(`${name} ${data[name]}`);
        }
    }

    printBoolean(data: any, name: string) {
        if (data && data[name] !== undefined) {
            this.writeln(`${name} ${data[name]}`);
        }
    }

    printNdk(ndk?: Ndk) {
        if (ndk) {
            this.block("ndk", () => {
                this.printString(ndk, "debugSymbolLevel");
                this.printString(ndk, "abiFilters");
            });
        }
    }

    printBuildConfigBody(buildConfig: BuildConfig) {
        this.printString(buildConfig, "applicationId");
        this.printNumber(buildConfig, "versionCode");
        this.printString(buildConfig, "versionName");
        if (buildConfig.signingConfig) {
            this.writeln(`signingConfig signingConfigs.${buildConfig.signingConfig}`);
        }
        this.printNumber(buildConfig, "minSdk");
        this.printNumber(buildConfig, "targetSdk");
        this.printString(buildConfig, "testInstrumentationRunner");
        this.printBoolean(buildConfig, "minifyEnabled");
        this.printBoolean(buildConfig, "multiDexEnabled");
        this.printExternalNativeBuild(buildConfig.externalNativeBuild);
        this.printNdk(buildConfig.ndk);

        if (buildConfig.proguardFiles) {
            this.printExpr(buildConfig, "proguardFiles");
        }
        if (buildConfig.multiDexKeepProguard) {
            this.printExpr(buildConfig, "multiDexKeepProguard");
        }
        if (buildConfig._extraCode) {
            for (const line of buildConfig._extraCode) {
                this.writeln(line);
            }
        }
    }

    printBundleConfig(bundle?: BundleConfig) {
        if (bundle) {
            this.block("bundle", () => {
                if (bundle.splitLanguage !== undefined) {
                    this.block("language", () => {
                        this.writeln(`enableSplit ${bundle.splitLanguage}`);
                    });
                }
                if (bundle.splitDensity !== undefined) {
                    this.block("density", () => {
                        this.writeln(`enableSplit ${bundle.splitDensity}`);
                    });
                }
                if (bundle.splitAbi !== undefined) {
                    this.block("abi", () => {
                        this.writeln(`enableSplit ${bundle.splitAbi}`);
                    });
                }
            });
        }
    }

    printSourceSets(sourceSets?: SourceSets) {
        if (sourceSets) {
            this.block("sourceSets", () => {
                for (const name of Object.keys(sourceSets)) {
                    this.block(name, () => {
                        const sourceSet = sourceSets[name];
                        for (const kind of Object.keys(sourceSet)) {
                            this.block(kind, () => {
                                const dirSet: SourceDirectorySet = sourceSet[kind];
                                this.printString(dirSet, "srcDirs");
                            });
                        }
                    });
                }
            });
        }
    }

    printAndroid(android?: AndroidConfig) {
        if (android) {
            this.block("android", () => {
                this.printNumber(android, "compileSdk");
                this.printString(android, "buildToolsVersion");
                this.printString(android, "ndkVersion");
                this.printSigningConfigs(android.signingConfigs);
                this.printCompileOptions(android.compileJavaVersion);
                this.printExternalNativeBuild(android.externalNativeBuild);
                if (android.defaultConfig) {
                    this.block("defaultConfig", () => {
                        this.printBuildConfigBody(android.defaultConfig);
                    });
                }
                if (android.buildTypes) {
                    this.block("buildTypes", () => {
                        for (const type of Object.keys(android.buildTypes)) {
                            this.block(type, () => {
                                this.printBuildConfigBody(android.buildTypes[type]);
                            });
                        }
                    });
                }
                this.printSourceSets(android.sourceSets);
                this.printBundleConfig(android.bundle);
            });
        }
    }

    printBaseModuleBody(config: BaseModuleConfig) {
        this.printPlugins(config.plugins);
        this.printRepositorySection(config.repositories);
        this.printAndroid(config.android);
        this.printDependencySection(config.dependencies);
    }

    printSigningConfigBody(config?: SigningConfig) {
        if (config) {
            if (config.storeFile) {
                this.writeln(`storeFile file('${config.storeFile}')`);
            }
            this.printString(config, "storePassword");
            this.printString(config, "keyAlias");
            this.printString(config, "keyPassword");
        }
    }

    printSigningConfigs(configs?: SigningConfigs) {
        if (configs) {
            this.block("signingConfigs", () => {
                for (const flavor of Object.keys(configs)) {
                    const config = configs[flavor];
                    if (config) {
                        this.block(flavor, () => {
                            this.printSigningConfigBody(config);
                        });
                    }
                }
            });
        }
    }

    save(filepath: string) {
        writeTextFileSync(filepath, this._buf);
    }
}

interface Repository {
    name: string;
    url?: string;
}

interface Dependency {
    type?: DependencyType;
    name: string;
    version?: string;
}

export interface BaseModuleConfig {
    plugins?: string[];
    repositories?: Repository[];
    dependencies?: (Dependency | string)[];
    android?: AndroidConfig;
}

export interface BuildTop {
    buildscript: BaseModuleConfig;
    /**
     * Settings applied to all modules. Currently only add repositories via `settings.gradle` (not `allprojects`)
     */
    allprojects: BaseModuleConfig;
}

export type AbiType = "armeabi-v7a" | "x86" | "arm64-v8a" | "x86_64";

interface Cmake {
    //  "../CMakeLists.txt"
    path?: string,
    //  "3.19.0+"
    version?: string

    // ANDROID_STL is c++_static by default
    // "-std=c++17"
    cppFlags?: string;
}

interface ExternalNativeBuild {
    cmake?: Cmake;
}

interface Ndk {
    // for example: 'armeabi-v7a', 'x86', 'arm64-v8a', 'x86_64'
    abiFilters?: AbiType[];
    debugSymbolLevel?: "SYMBOL_TABLE" | "FULL";
}

// defaultConfig {
export interface BuildConfig {
    applicationId?: string;
    versionCode?: number;
    versionName?: string;
    /**
     * Signing config name from `android.signingConfigs`
     */
    signingConfig?: string;
    minSdk?: number;
    targetSdk?: number;
    multiDexEnabled?: boolean;
    testInstrumentationRunner?: string;

    externalNativeBuild?: ExternalNativeBuild;
    ndk?: Ndk;

    minifyEnabled?: boolean;
    // getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
    proguardFiles?: string[];

    //  file('multidex-config.pro')
    multiDexKeepProguard?: string[];

    /**
     * Additional lines with code will be printed at the end of closure
     */
    _extraCode?: string[];
}

export interface SourceDirectorySet {
    srcDirs?: string[];
}

export type SourceSet = { [src: string]: SourceDirectorySet };
export type SourceSets = { [name: string]: SourceSet };

export interface BundleConfig {
    splitLanguage?: boolean;
    splitDensity?: boolean;
    splitAbi?: boolean;
}

export interface AndroidConfig {
    compileSdk: number;
    buildToolsVersion: string;
    ndkVersion: string;
    signingConfigs?: SigningConfigs;
    defaultConfig: BuildConfig;
    buildTypes: { [buildType: string]: BuildConfig };
    externalNativeBuild?: ExternalNativeBuild;
    sourceSets?: SourceSets;
    compileJavaVersion?: string;
    bundle?: BundleConfig;
}

interface SigningConfig {
    keyAlias?: string;
    keyPassword?: string;
    storeFile?: string;
    storePassword?: string;
}

export type SigningConfigs = { [flavor: string]: SigningConfig };