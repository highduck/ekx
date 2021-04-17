import {Project} from "../project";
import * as path from "path";
import {executeAsync, makeDirs} from "../utils";
import {buildAssetsAsync} from "../assets";
import {rmSync} from "fs";

const exportDir = "export/uitest";

interface DisplayInfo {
    size: number[];
    insets: number[];
    relative?: boolean;
}

const displays: { [key: string]: DisplayInfo } = {
    Android_GooglePlay: {
        size: [9, 16],
        insets: [0, 0, 0, 0],
        relative: true
    },
    // app store reference: https://appradar.com/blog/ios-app-screenshot-sizes-and-guidelines-for-the-apple-app-store
    iPhone_6_5_inch: {
        // iPhone 6.5 inch
        // - iPhone 11 Pro Max : 1242 x 2688 [0, 132, 0, 102]
        // - iPhone 11 : 828 x 1792 [0, 96, 0, 68]
        // - iPhone XS Max : N/A
        // - iPhone XR : N/A
        size: [1242, 2688],
        insets: [0, 132, 0, 102]
    },
    iPhone_5_5_inch: {
        // iPhone 5.5 inch
        // - iPhone 6s Plus : N/A
        // - iPhone 7 Plus : N/A
        // - iPhone 8 Plus : 1242 x 2208 [0, 0, 0, 0]
        size: [1242, 2208],
        insets: [0, 0, 0, 0]
    },
    iPad_12_9_inch_2rd_gen: {
        // iPad 12.9 inch (2nd generation iPad Pro)
        // - 2048 x 2732 [0, 0, 0, 0]
        size: [2048, 2732],
        insets: [0, 0, 0, 0]
    },
    iPad_12_9_inch_3rd_gen: {
        // iPad 12.9 inch (3rd generation iPad Pro)
        // - 2048 x 2732 [0, 0, 0, 40]
        // * iPad Pro 12.9-inch (4th gen) : 2048 x 2732 [0, 0, 0, 40]
        size: [2048, 2732],
        insets: [0, 0, 0, 40]
    },
};

async function build() {
    const buildDir = path.join(exportDir, "cmake-build-release");
    const args = [
        "-S", ".",
        "-B", buildDir,
        "-G", "Ninja",
        `-DCMAKE_BUILD_TYPE=Release`,
        "-DEK_UITEST=ON"
    ];

    args.push("-DCMAKE_C_COMPILER_LAUNCHER=ccache", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache");

    await executeAsync("cmake", args);
    await executeAsync("cmake", [
        "--build", buildDir
    ]);
}

function doScreenshots(ctx: Project): Promise<any> {
    const bin = path.join(exportDir, ctx.binary_name);
    const jobs = [];
    const screenshotsDir = path.join(exportDir, "screenshots");
    rmSync(screenshotsDir, {recursive: true, force: true});
    let wndi = 0;

    for (const sim of Object.keys(displays)) {
        const display = displays[sim];
        for (const lang of ["en", "ru"]) {
            const screenshotOutput = path.join("screenshots", sim, lang);
            const outputDir = path.join(screenshotsDir, sim, lang);
            makeDirs(outputDir);

            const windowSettings = [0, 0, 200, 200];
            windowSettings[0] = 200 + 200 * (jobs.length % 5);
            windowSettings[1] = 200 + 200 * Math.floor(jobs.length / 5);
            const args = [
                "--uitest", "screenshots",
                "--screenshot-output", screenshotOutput + "/",
                "--lang", lang,
                "--display", display.size.concat(display.insets).concat([display.relative ? 1 : 0]).join(","),
                "--window", windowSettings.join(",")
            ];
            const job = executeAsync(path.resolve(bin), args, {
                workingDir: path.join(process.cwd(), exportDir),
                verbose: true
            });
            jobs.push(job);
        }
    }
    return Promise.all(jobs);
}

export async function screenshots(ctx: Project): Promise<any> {
    await Promise.all([
        buildAssetsAsync(ctx, path.join(exportDir, "assets"), true),
        build()
    ]);
    await doScreenshots(ctx);
}