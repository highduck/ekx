---
"@ekx/ekx": patch
---

- audio: defer device start
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