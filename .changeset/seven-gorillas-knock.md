---
"@ekx/ekx": patch
---

- core: rand api changed, randomized from time with offset by default
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