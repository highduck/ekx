# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

- fix android remove ads purchase restore and already owned error handling

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

- Initial release as is
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

[Unreleased]: https://github.com/eliasku/ekx/compare/0.0.1...HEAD
[0.0.1]: https://github.com/eliasku/ekx/releases/tag/0.0.1