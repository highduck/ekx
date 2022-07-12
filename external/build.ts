import box2d from "./box2d/recipe.ts";
import dr_libs from "./dr-libs/recipe.ts";
import cairo from "./cairo/recipe.ts";
import dev_fonts from "./dev-fonts/recipe.ts";
import freetype from "./freetype/recipe.ts";
import stb from "./stb/recipe.ts";
import sokol from "./sokol/recipe.ts";
import quickjs from "./quickjs/recipe.ts";
import pugixml from "./pugixml/recipe.ts";
import miniz from "./miniz/recipe.ts";
import imgui from "./imgui/recipe.ts";

type LibFn = undefined | (() => (undefined | void | Promise<void>));

interface Lib {
    clean?: LibFn;
    fetch?: LibFn;
    test?: LibFn;
}

const libs: { [key: string]: Lib } = {
    box2d,
    dr_libs,
    cairo,
    dev_fonts,
    freetype,
    stb,
    sokol,
    quickjs,
    pugixml,
    miniz,
    imgui,
};

async function runSerialTasks(task: string) {
    for (const name of Object.keys(libs)) {
        const lib: Lib = libs[name];
        if (task in lib && (lib as any)[task] != null) {
            const taskFn = (lib as any)[task];
            if (taskFn != null) {
                try {
                    const promise = taskFn();
                    if (promise instanceof Promise) {
                        await promise.catch(e => {
                            console.error(`${task} failed for ${name}:`, e);
                        });
                    }
                } catch (e) {
                    console.error(e);
                }
            }
        }
    }
}

function runParallelTasks(task: string) {
    const tasks = [];
    for (const name of Object.keys(libs)) {
        const lib: Lib = libs[name];
        if (task in lib && (lib as any)[task] != null) {
            const taskFn = (lib as any)[task];
            if (taskFn != null) {
                try {
                    const promise = taskFn();
                    if (promise instanceof Promise) {
                        tasks.push(promise.catch(e => {
                            console.error(`${task} failed for ${name}:`, e);
                        }));
                    }
                } catch (e) {
                    console.error(e);
                }
            }
        }
    }
    return Promise.allSettled(tasks);
}

await runParallelTasks("clean");
await runSerialTasks("fetch");
await runSerialTasks("test");
