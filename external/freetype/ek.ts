import {Project} from "../../modules/cli/project.js";

export function setup(project:Project) {
    project.addModule({
        name: "freetype",
        cpp_include: ["include", "src"],
        // TODO: to implement cpp_sources
        cpp_sources: [
            "src/base/ftadvanc.c",
            "src/base/ftcalc.c",
            "src/base/ftcolor.c",
            "src/base/ftdebug.c",
            "src/base/fterrors.c",
            "src/base/ftfntfmt.c",
            "src/base/ftgloadr.c",
            "src/base/fthash.c",
            "src/base/ftlcdfil.c",
            "src/base/ftobjs.c",
            "src/base/ftoutln.c",
            "src/base/ftpsprop.c",
            "src/base/ftrfork.c",
            "src/base/ftsnames.c",
            "src/base/ftstream.c",
            "src/base/fttrigon.c",
            "src/base/ftutil.c",

            "src/base/ftinit.c",
            "src/base/ftglyph.c",
            "src/base/ftsystem.c",
            "src/base/ftbitmap.c",
            "src/base/ftsynth.c",

            "src/autofit/autofit.c",
            "src/bdf/bdf.c",
            "src/bzip2/ftbzip2.c",
            "src/cache/ftcache.c",
            "src/cff/cff.c",
            "src/cid/type1cid.c",
            "src/gzip/ftgzip.c",
            "src/lzw/ftlzw.c",
            "src/pcf/pcf.c",
            "src/pfr/pfr.c",
            "src/psaux/psaux.c",
            "src/pshinter/pshinter.c",
            "src/psnames/psnames.c",
            "src/raster/raster.c",
            "src/sfnt/sfnt.c",
            "src/smooth/smooth.c",
            "src/truetype/truetype.c",
            "src/type1/type1.c",
            "src/type42/type42.c",
            "src/winfonts/winfnt.c",
            "src/sdf/sdf.c"
        ]
    });
}
