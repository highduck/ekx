# Changelog

## 0.1.5

### Patch Changes

- 84239f3: minor api renaming
- 29b4dd3: - core: rand api changed, randomized from time with offset by default
  - scenex: basic application methods renaming, systems go by default
  - core: Res utility assign function to copy POD structures into slot
  - Trail2D: fix scale, fix rotated sprite, tune defaults
  - Display2D: make function with arguments for Drawable2D constructor
  - Display2D: add static `tryGet` function
  - Particle2D: add alpha modes DCBlink and QuadOut, add Emitter velocity, add helper functions
  - inspector: add Viewport component view
  - scenex: remove Camera2D relative viewport settings, use Viewport setup
  - scenex: update all viewports by base application (display data owner)
  - particles: fix emitter-layer space transform
  - scenex: interactive bubble mode by default
  - interaction system camera order fix, capture camera and world position to Interactive component
  - changed hit test order for Bounds2D hit-area to bottom layer
  - add current hit target path to Stats window
- 1a74d06: update dependencies

## 0.1.4

### Patch Changes

- 0c89a5d: - audio: defer device start
  - ios: fix logging
  - timers: fix executing queue
  - app arguments: changed to be static to allow fill before app module initialization
  - ios/macos: fix preferred language detection
  - macos: add --window flag to allow set window position and size
  - rename `hook_on_draw_frame` to `onRenderOverlay`
  - inspector: fix loading font from general assets, should be exported with `--dev` switch
  - audio: remove update polling and do not set sound properties if not changed
  - draw2d: add support for offscreen depth-stencil target
  - draw2d: add setup main render targets
  - graphics: add `Texture::getPixels` method for Metal backend (to make screenshots on macOS)
  - changes for new initialization and preloading flow
  - ios: fix Metal default render pass reference
  - app arguments utility moved to core module
- 0c89a5d: add debug logging on subsystems initialization

## 0.1.3

### Patch Changes

- e79c61c: minor update
- d90b3e3: fix audio pause/resume on switching app to background
- b985463: change GL to Metal for macOS and iOS
- dee7ff1: update ios/mac app, sg migration to homemade Array

## 0.1.2

### Patch Changes

- f7692d8: support USE_CCACHE

## 0.1.1

### Patch Changes

- 67881dd: ecxx: api changes
- f14659f: gpgs: fix signin and submitt scores
- 566016e: update dependencies

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres
to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.0.5] 2021-03-13

- change preferences api to c-strings
- fix compile errors
- [android] update project dependencies

## [0.0.4]

### graphics

- change `alpha8` format more portable (R8/LUMINANCE)
- add default program `2d_alpha`
- unify 2d shaders
- change vertex color to non-premultiplied, now calculate pre-multiplication on vertex shader

### appbox / goodies

- buttons: reduce buttons shaking
- popups: fade tween on last popup closing

### scenex

- Display2D: program state, `2d_color` shader for large background colored quads (reduce gpu load)
- Profiler: add non-clamped Fill-Rate stats

### Tools

- Remove `{fmt}` and `json` libraries
- Add command to prerender flash symbols via command-line
- Changed ios app icon generation
- iOS: fix project generation script, compile fixes
- iOS: update and fix `mod-pbxproj` to `3.2.0`

## [0.0.3] - 2020-11-22

- hotfix android remove ads purchase restore and already owned error handling

## [0.0.2] - 2020-11-22

- remove `Particle` cycled mode
- add `DynamicAtlas` multiple pages, mipmaps, alpha8 / color32
- improvements for editor
- add importers for `Atlas` and `DynamicAtlas`
- fix resize and scale for any orientation
- android: relax requirements for device orientation
- ecxx: entity methods
- fix mouse cursor
- refactor `Transform2D` component
- changed BitmapFont typename to `bmfont`
- merge `name_t` and `node_state_t` to `Node`
- update ImGui v1.80
- fix macOS backspace/delete key codes
- [audio] change api to Sound and Music instances
- [audio] change `simple_audio_manager` to `AudioManager`
- add `TrueTypeFont`
- remove multiple sizes for bitmap font export
- [demo] update `ek-demo` project
- [graphics] get context type
- add `alpha8` texture format

## [0.0.1] - 2020-10-31

- Initial releaseNode as is
- start versioning and CHANGELOG
- update play-services-base 17.5.0
- [ecxx] remove template parameter for `sparse_vector`
- [signal] change emplace_back to push_back
- [core] add javascript style timers: setTimeout, setInterval, clearTimeout, clearInterval utility functions
- [audio] remove `pan` parameter
- [audio] mute/unmute, update miniaudio and raudio libraries, remove old ek-audiomini(inhouse+cocos) plugin
- [android] change name EkExtension to EkPlugin
- [android] fix bug with event_nop which leads to resume event is not handled
- [mini-ads] deprecated: should be replaced by admob+billing plugins
- [admob] initial plugin (ios, android, null)
- [billing] initial plugin (ios, android, null)

[unreleased]: https://github.com/eliasku/ekx/compare/0.0.1...HEAD
[0.0.1]: https://github.com/eliasku/ekx/releases/tag/0.0.1
