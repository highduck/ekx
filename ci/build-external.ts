import box2d from "../external/box2d/recipe.js";
import cairo from "../external/cairo/recipe.js";
import freetype from "../external/freetype/recipe.js";
import headers from "../external/headers/recipe.js";
import sokol from "../external/sokol/recipe.js";
import quickjs from "../external/quickjs/recipe.js";
import pugixml from "../external/pugixml/recipe.js";
import miniz from "../external/miniz/recipe.js";
import imgui from "../external/imgui/recipe.js";

type LibFn = undefined | (() => (undefined | void | Promise<void>));

interface Lib {
    clean?: LibFn;
    fetch?: LibFn;
    test?: LibFn;
}

const libs: { [key: string]: Lib } = {
    headers,
    box2d,
    cairo,
    freetype,
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
