#!/usr/bin/env bash

../external/sokol/bin/osx/sokol-shdc -i ek/draw2d/draw2d.glsl -o ek/draw2d/draw2d_shader.h -l glsl330:glsl300es:glsl100 -b
../external/sokol/bin/osx/sokol-shdc -i ek/scenex/3d/render3d.glsl -o ek/scenex/3d/render3d_shader.h -l glsl330:glsl300es:glsl100 -b
