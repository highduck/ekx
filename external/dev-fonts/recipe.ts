import {path} from "ekx/deps.ts"
import {downloadFiles, getModuleDir, rm} from "ekx/utils/mod.ts"
import {buildMatrix} from "ekx/cmake/mod.ts"

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "build"));
    await rm(path.join(__dirname, "include"));
    await rm(path.join(__dirname, "ttf"));
}

function fetch():Promise<void> {
    console.info("dev-fonts");

    const tasks = [
        downloadFiles({
            srcBaseUrl: `https://github.com/ocornut/imgui/raw/docking`,
            destPath: path.join(__dirname, "ttf"),
            fileMap: {
                "misc/fonts/Cousine-Regular.ttf": "Cousine-Regular.ttf"
            }
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/FortAwesome/Font-Awesome/raw/master/webfonts`,
            destPath: path.join(__dirname, "ttf"),
            fileList: [
                "fa-regular-400.ttf",
                "fa-solid-900.ttf"
            ]
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/google/material-design-icons/raw/master/font`,
            destPath: path.join(__dirname, "ttf"),
            fileList: [
                "MaterialIcons-Regular.ttf"
            ]
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/juliettef/IconFontCppHeaders/raw/main`,
            destPath: path.join(__dirname, "include"),
            fileList: [
                "IconsFontAwesome5.h",
                "IconsMaterialDesign.h"
            ]
        })
    ];
    return Promise.all(tasks).then(()=>undefined);
}

async function test() {
    await buildMatrix({
        cmakePath: "test",
        test: true,
        workingDir: __dirname
    });
    await rm(path.join(__dirname, "build"));
}

export default {clean, fetch, test};
