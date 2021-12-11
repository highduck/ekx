const {downloadFiles} = require('@ekx/cli-utils');

async function run() {
    console.info("sokol");
    await downloadFiles({
        srcBaseUrl: "https://github.com/floooh/sokol/raw/master",
        destPath: "src",
        fileList: [
            "sokol_app.h",
            "sokol_args.h",
            "sokol_audio.h",
            "sokol_fetch.h",
            "sokol_gfx.h",
            "sokol_glue.h",
            "sokol_time.h",
            "util/sokol_debugtext.h",
            "util/sokol_fontstash.h",
            "util/sokol_gfx_imgui.h",
            "util/sokol_gl.h",
            "util/sokol_imgui.h",
            "util/sokol_memtrack.h",
            "util/sokol_shape.h",
        ]
    });
}

run().catch(() => process.exit(1));