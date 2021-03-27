# ekx

## Key Features

- *Edit Mode* 
    - Resources *hot reloading*
    - Resources *loading on the fly*
    - Resources *deferred export*
    - Sprites resolution hot reloading on Window Resize
    - Export Platforms (iOS, Android, Web)

- Export for Production
    - Build statically against the whole Codebase
    - Run/Debug in XCode and Android Studio
    - Debug runtime-code
    - Stripped libraries code
    
- Capabilities
    - ECS framework
    - Multi-resolution export: @x1, @x2, @x3, @x4
    - Export from Animate CC (`.fla` / `.xfl` files)
    - Import TTF fonts with Effects
    - GLSL shaders
    - Marketing assets export from Flash template
    - Simple Binary serialization

## Tasks

[Trello Board](https://trello.com/b/MW9eYAOA/ekx)

## Prerequisites for tooling

- `python3` and `pip`
- NodeJS, NPM
- CMake >= 3.13
- XCode for iOS
- Android Studio for Android

## Dependencies

#### System packages

```
brew install pkg-config
brew install cairo
```

- pkg-config
- Cairo
- Freetype2

# Development

It's very hard to fix platform-side code inside generated projects (XCode, Android Studio), so
we able to switch platform inside your cmake-compatible IDE (like CLion).

1. Create Debug-Emscripten
2. Set the path to CMake Emscripten Toolchain

```shell script
# Emscripten
-DCMAKE_TOOLCHAIN_FILE=/Users/PATH_TO_emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

# iOS
-DCMAKE_TOOLCHAIN_FILE=$EKX_ROOT/external/cmake/ios.toolchain.cmake

# Android
-DCMAKE_TOOLCHAIN_FILE=/Users/ilyak/Library/Android/sdk/ndk/20.0.5594570/build/cmake/android.toolchain.cmake -DANDROID_ABI=x86_64

# Windows from MacOS
brew install mingw-w64
-DCMAKE_TOOLCHAIN_FILE=$EKX_ROOT/external/cmake/x86_64-w64-mingw32.cmake
```

3. Set Env from:
    For example you have output of running `emsdk_env.sh`:
```sh
PATH += /Users/ilyak/dev/emsdk
PATH += /Users/ilyak/dev/emsdk/upstream/emscripten
PATH += /Users/ilyak/dev/emsdk/node/12.9.1_64bit/bin

EMSDK = /Users/ilyak/dev/emsdk
EM_CONFIG = /Users/ilyak/.emscripten
EMSDK_NODE = /Users/ilyak/dev/emsdk/node/12.9.1_64bit/bin/node
```
So add the PATH and EM variables:

```
PATH = /Users/ilyak/dev/emsdk;/Users/ilyak/dev/emsdk/upstream/emscripten;/Users/ilyak/dev/emsdk/node/12.9.1_64bit/bin;$PATH
EMSDK = /Users/ilyak/dev/emsdk
EM_CONFIG = /Users/ilyak/.emscripten
EMSDK_NODE = /Users/ilyak/dev/emsdk/node/12.9.1_64bit/bin/node
```