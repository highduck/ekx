## 0.0.4

## 0.1.15

### Patch Changes

- 8e787084: add exit code on error
- Updated dependencies [cf6702f5]
  - @ekx/scenex@0.1.11
  - @ekx/obj-export@0.0.4
  - @ekx/bmfont-export@0.0.5
  - @ekx/sprite-packer@0.0.5
  - @ekx/flash-export@0.0.10

## 0.1.14

### Patch Changes

- Updated dependencies [fd8f9b35]
  - android-proj-gen@0.0.8
  - @ekx/scenex@0.1.10

## 0.1.13

### Patch Changes

- Updated dependencies [27598eb6]
  - @ekx/bmfont-export@0.0.4
  - @ekx/flash-export@0.0.9
  - @ekx/sprite-packer@0.0.4

## 0.1.12

### Patch Changes

- 53b259fe: android: pass `cpp_flags` to generated AS project
- b7fc9336: refactoring
- 8bd2c87b: Bump @types/node from 16.11.13 to 17.0.2
- c2684e0e: Bump @types/node from 17.0.2 to 17.0.4
- b7fc9336: generate C-compat `build_info.h` instead of C++ version with namespace and const
- 326e845d: upgrade deps
- bf4d88e8: ios: improve generated xcode project structure
- a08fb558: Bump @types/node from 16.11.12 to 16.11.13
- Updated dependencies [4349a862]
- Updated dependencies [4349a862]
- Updated dependencies [b7fc9336]
- Updated dependencies [8bd2c87b]
- Updated dependencies [c2684e0e]
- Updated dependencies [326e845d]
- Updated dependencies [26fa74d6]
- Updated dependencies [1e2a322a]
- Updated dependencies [a08fb558]
  - @ekx/flash-export@0.0.8
  - android-proj-gen@0.0.7
  - @ekx/scenex@0.1.9
  - @ekx/bmfont-export@0.0.3
  - cmake-build@0.0.5
  - @ekx/obj-export@0.0.3
  - @ekx/sprite-packer@0.0.3

## 0.1.11

### Patch Changes

- e3922169: cli: change workflow for version, build-number and name, now it's controlled by project's package.json file, native BuildInfo.h generates on each build step
- 11a94ae7: android: remove project template (use android-proj-gen package)
- d75fd9cf: Bump http-server from 13.0.1 to 13.0.2
- 7506f0a5: change vibrate implementation to auph library
- 4759a4ff: up dep versions
- 7506f0a5: web: update html template for MINIMAL_RUNTIME
- 85403373: ios: fix default assets, add debug symbols for Crashlytics
- 11a94ae7: android: enable all native strip features
- 3609031d: refactoring
- bee52e67: adnroid: remove required release signing options, add application_id and package_id by default
- 404b1bda: Bump typescript from 4.4.2 to 4.4.3
- 7506f0a5: sync @ekx api changes
- af268fad: Bump @types/node from 16.9.1 to 16.9.2
- b00505af: app module refactoring
- Updated dependencies [923f15f8]
- Updated dependencies [4759a4ff]
- Updated dependencies [67dc93d6]
- Updated dependencies [3609031d]
- Updated dependencies [cac14838]
- Updated dependencies [82f3ee44]
- Updated dependencies [7506f0a5]
- Updated dependencies [7506f0a5]
  - @ekx/sprite-packer@0.0.2
  - @ekx/bmfont-export@0.0.2
  - @ekx/scenex@0.1.8
  - @ekx/flash-export@0.0.7
  - @ekx/obj-export@0.0.2
  - android-proj-gen@0.0.6
  - cmake-build@0.0.4

## 0.1.10

### Patch Changes

- 42f3ca4: Bump typescript from 4.3.5 to 4.4.2

## 0.1.9

### Patch Changes

- 55ddf9a: move assets packing to js
- 7accc61: switch to cmake-build
- 55ddf9a: update android deps
- 55ddf9a: fix web service worker to update app
- Updated dependencies [de912c6]
- Updated dependencies [55ddf9a]
- Updated dependencies [7accc61]
- Updated dependencies [55ddf9a]
- Updated dependencies [55ddf9a]
  - @ekx/ekx@0.1.7
  - @ekx/ekc@0.1.9

## 0.1.8

### Patch Changes

- a70fd7c: dev: add SF-Mono font to development tools
- 5db3c48: web: add --run option
- 4bcbd1e: update dependencies and modules configuration
- aa06a67: cli: ignore missing pngquant
- Updated dependencies [93b2ab4]
- Updated dependencies [f23d368]
- Updated dependencies [a70fd7c]
- Updated dependencies [4bcbd1e]
- Updated dependencies [d4b6523]
- Updated dependencies [4bcbd1e]
- Updated dependencies [5db3c48]
  - @ekx/ekc@0.1.8
  - @ekx/ekx@0.1.6

## 0.1.7

### Patch Changes

- 84239f3: minor api renaming
- 84239f3: Fix web app icons and manifest generation
- 1a74d06: update dependencies
- Updated dependencies [84239f3]
- Updated dependencies [29b4dd3]
- Updated dependencies [1a74d06]
  - @ekx/ekx@0.1.5
  - @ekx/ekc@0.1.7

## 0.1.6

### Patch Changes

- Updated dependencies [f19bfcd]
  - @ekx/ekc@0.1.6

## 0.1.5

### Patch Changes

- Updated dependencies [b113128]
  - @ekx/ekc@0.1.5

## 0.1.4

### Patch Changes

- deebbb2: - add dev assets
  - add `updateGoogleServicesConfig` command to fetch firebase project configuration changes
  - add `screenshots` command to build and screenshot run ui test script
  - add support async command execution
  - ios template: remove project renaming
- Updated dependencies [0c89a5d]
- Updated dependencies [deebbb2]
- Updated dependencies [0c89a5d]
  - @ekx/ekx@0.1.4
  - @ekx/ekc@0.1.4

## 0.1.3

### Patch Changes

- e79c61c: minor update
- b985463: change GL to Metal for macOS and iOS
- d90b3e3: bump version code for any kind of deployment
- Updated dependencies [e79c61c]
- Updated dependencies [d90b3e3]
- Updated dependencies [b985463]
- Updated dependencies [dee7ff1]
  - @ekx/ekx@0.1.3
  - @ekx/ekc@0.1.3

## 0.1.2

### Patch Changes

- Updated dependencies [f9b7ca4]
- Updated dependencies [f7692d8]
  - @ekx/ekc@0.1.2
  - @ekx/ekx@0.1.2

## 0.1.1

### Patch Changes

- 65d2b1a: android template: upgrade dependencies
- f14659f: fix web service worker caching, web manifest icons, versions switch, deploy internal testing
- Updated dependencies [67881dd]
- Updated dependencies [f14659f]
- Updated dependencies [566016e]

  - @ekx/ekx@0.1.1
  - @ekx/ekc@0.1.1

- add `ekx android bundle` to build releaseNode Android Application Bundle file to `export/` folder

## 0.0.2

- android: debug/releaseNode signing options
- fix adding modules
