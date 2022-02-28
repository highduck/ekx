import replace from '@rollup/plugin-replace';

function createGlobalPlugins(release) {
    return [
        replace({
            preventAssignment: true,
            values: {
                "process.env.NODE_ENV": JSON.stringify(release ? "production" : "development")
            }
        })
    ];
}

export default [
    {
        input: "./web/dist/module/webaudio/index.js",
        output: {
            // support core api for emscripten implementation
            file: "./web/dist/emscripten/auph.js",
            format: "iife",
            name: "auph",
            compact: true,
            sourcemap: true
        },
        plugins: createGlobalPlugins(true)
    }
];