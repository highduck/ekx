# cmake-build

[![Build](https://github.com/eliasku/cmake-build/actions/workflows/build.yml/badge.svg)](https://github.com/eliasku/cmake-build/actions/workflows/build.yml)
[![Version](https://img.shields.io/npm/v/cmake-build)](https://www.npmjs.com/package/cmake-build)
[![Downloads](https://img.shields.io/npm/dw/cmake-build)](https://www.npmjs.com/package/cmake-build)

Utility for configuring and building CMake projects with NPM packages support. Basic cross-compile features are provided (building for another system from a host system).

## CMake NPM packages

Link CMake projects installed from NPM packages
```cmake
include(npm.cmake)
```

- Add `cmake-build` to your `package.json` in `dependencies` section (not `devDependencies` because we want to resolve dependencies after install automatically)
- Add `include(npm.cmake)` to the beginning of your `CMakeLists.txt`
- Run `npx cmake-build` to generate configuration module `npm.cmake` with resolved NPM dependencies

### Package name convention

- Scoped NPM package name `@scope/package-name` transforms to CMake target `scope::package-name`
- Packages without scope is not transform

## Cross-compile

### Status

- [x] MacOS
  - [x] Android
  - [x] WebAssembly
  - [x] iOS
  - [x] macOS
  - [x] Windows (Mingw)
  - [x] Linux (Docker)
- [ ] Windows
- [ ] Linux

### Requirements

- [Emscripten SDK](https://emscripten.org/) for WebAssembly target
- [Android Studio](https://developer.android.com/studio) or **Android NDK** for Android target

#### macOS

Install `Mingw` for building Windows platform.

```shell
brew install mingw-w64
```

Install `docker` and follow guides how to build from images.

### IDE

It's very hard to fix platform-side code inside generated projects (XCode, Android Studio), so
we able to switch platform inside your cmake-compatible IDE (like CLion).

1. Create Debug-Emscripten
2. Set the path to CMake Emscripten Toolchain

```shell
# Emscripten
-DCMAKE_TOOLCHAIN_FILE=/Users/PATH_TO_emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

# iOS (https://github.com/leetal/ios-cmake/blob/master/ios.toolchain.cmake)
-DPLATFORM=SIMULATOR64 -DCMAKE_TOOLCHAIN_FILE=./cmake/ios.cmake

# Android
-DANDROID_ABI=x86_64 -DCMAKE_TOOLCHAIN_FILE=/Users/USER/Library/Android/sdk/ndk/23.0.7599858/build/cmake/android.toolchain.cmake

# Windows from MacOS
brew install mingw-w64
-DCMAKE_TOOLCHAIN_FILE=./cmake/mingw-w64-x86_64.cmake
```

### Docker

Docker files for cross-platform building

```shell
# build docker image for linux build
cd images/linux
docker build -t eliasku/ubuntu:1.0 .

# run docker in your current project dir
cd your/project/path
docker run -it --mount type=bind,source=$(pwd),target=/mnt eliasku/ubuntu:1.0 /bin/sh -c "cd /mnt && bash"
```