# ekx tasks

- better names for box/rect, sphere/circle?
- [prefs] change int to number as the most common case

Integrations:
- Chipmunk 2D would be nice
- Scripting Lua

Particles Designer:
- ?

iOS project Generator: 
- pass list of dependencies for SRC folders and HEADER base-path to Python generator scritps

## Audio packages

#### Old Audio separation:
1. Decouple and rename current audio package to cocos-audio?
2. DROP. Replace Android implementation with cocos?
3. DROP. Make and test Emscripten Howl / cocos-js impl?

#### New Audio implementation goals:
1. decouple audio to separated ek-module
2. connect miniaudio.h backend + mp3 support: https://github.com/dr-soft/miniaudio
3. look raylib audio implementation for controlling sounds: https://github.com/raysan5/raylib/blob/master/src/raudio.c

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

