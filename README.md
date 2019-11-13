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

## Prerequisites for tooling

- `python3` and `pip`
- Conan Package Manager
- CMake >= 3.12
- XCode for iOS
- Android Studio for Android

## Dependencies

### Output Mode Dependencies

Game output dependencies are required to be source packed in.

Platform requirements are Frameworks or Libraries are resolved later when you build final version.
 TODO: examples   

#### Built-in
- `stb_image` (included)

### Edit Mode Dependencies

#### System packages

```
brew install pkg-config
brew install cairo
```

- pkg-config
- Cairo
- Freetype2

#### Built-in
- `miniz` and `zip_file.cpp` (patched)
- `stb_image_write` (included)

#### External packages
- `imgui` !
- `pugixml`

- `fmt`
- `json_for_modern_cpp`

#### Tooling dependencies
- [will be removed] `mod-pbxproj` for XCode project patching

### Development requirements

- cmake >= 3.12
- modern c++17 compiler
- NodeJS (ver >=12), NPM
- EMSDK
- [could be removed] python3

## Installation

```bash
pip3 install --upgrade conan
conan remote add -f eliasku https://api.bintray.com/conan/eliasku/public-conan
conan install -if cmake-build-debug -pr clion-debug -b missing .
```

add tooling root to environment
```
open -e ~/.bash_profile
```

add lines:
```
source ~/highduck/ekx/env.sh
```


## TODO

- Windows support
- Audio
- Repack Atlases by Tag


# Development

CLion:
1. Create Debug-Emscripten
2. Set the path to CMake Emscripten Toolchain

`-DCMAKE_TOOLCHAIN_FILE=/Users/PATH_TO_emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake`

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