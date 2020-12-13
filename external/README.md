# Dependencies

## Client Stuff

### [stb](https://github.com/nothings/stb)

Package: manual from github

`stb_image`

- **CLIENT** `ek-core` decodes PNG and JPEG images

`stb_truetype`

- **CLIENT** `scenex` rendering TrueType fonts

### [miniaudio](https://github.com/mackron/miniaudio)

Package: manual from github

- **CLIENT** cross-platform audio device abstraction

### [dr_libs](https://github.com/mackron/dr_libs)

Package: manual from github

`dr_mp3`

- **CLIENT** decoding MP3 file resources

## Only for Editor

### [stb](https://github.com/nothings/stb)

Package: manual from github

`stb_image`

- `ek-flash` decodes JPEG bitmap data from XFL

`stb_truetype`

- `ek-editor` header for `dear-imgui`

`stb_image_write`

- `ek-flash` sprite-pack use it to save atlas images
- `ek-editor` for realtime screen recording (screenshots)

`stb_rect_pack`

- `ek-editor` header for `dear-imgui`

### [pugixml](https://github.com/zeux/pugixml)

Package: manual from github

### [miniz](https://github.com/richgel999/miniz)

Package: manual from github

(!) **versions is fixed**

### [Dear ImGui](https://github.com/ocornut/imgui)

Package: manual from github

### Cairo

Package: CMake module included

(!) Need install to the system

- `ek-flash` renders vector graphics

### FreeType2

Package: CMake FindFreetype

(!) Need install to the system

- `ek-flash` exports bitmap fonts