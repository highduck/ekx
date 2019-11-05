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

Production Mode:
- `stb_image` (included)

Edit Mode:
- `imgui` !
- `pugixml`
- `cairo`
- `freetype2`
- `stb_image_write` (included)
- `fmt`
- `kainjow-mustache`
- `json_for_modern_cpp`
- `mod-pbxproj` python library
- `miniz` and `zip_file.cpp` (patched)

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

ReleaseWasm
`-DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/Users/ilyak/dev/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake`

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/Users/ilyak/dev/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
