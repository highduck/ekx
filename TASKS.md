# ilj features

- virtual atlas asset, need to merge sprites while importing bitmap font and flash files
- rename asset type `freetype` to `bitmapfont`
- enable atlas mipmaps? Support bias in shaders

TrueType fonts
- meta information for atlas page size, base font size
- outlines/shadows support
- shared atlas between fonts (?)
- enable mipmaps (?)
- fallback to system fonts (?)

Text rendering
- UTF-8 reading to codepoints
- line spacing, letter spacing
- line-endings
- line alignments
- fit auto-size
- text wrapping
- styling: font, size, color
- underline, strikethrough

# ekx tasks

- better names for box/rect, sphere/circle?
- [prefs] change int to number as the most common case

Integrations:
- Chipmunk2D or Box2D would be nice
- Scripting Lua

Particles Designer:
- ?

iOS project Generator: 
- pass list of dependencies for SRC folders and HEADER base-path to Python generator scritps

## DONE

+ check side-project issues
+ side-projects: Create event-name string constant (for example "back_button")!
+ ek::draw2d::
+ ek::audio_mini::
+ ek::graphics::
+ remove Forward declarations from math (matrix, vec, box)
+ remove scenex/ecs/config
+ fix button_skin field namings
+ sfx to pico-sample
+ fix no-rtti easing

