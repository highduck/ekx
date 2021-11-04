import {terser} from 'rollup-plugin-terser';
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
        input: "./web/dist/module/lib.js",
        output: {
            file: "./platform/web/lib/lib-texture-loader.js",
            format: "es",
            compact: true,
            module: false,
            //plugins: [terser()],
            sourcemap: true
        },
        plugins: createGlobalPlugins(true)
    },
    {
        input: "./web/dist/module/index.js",
        output: {
            file: "./platform/web/pre/texture-loader.js",
            format: "iife",
            name: "TextureLoader",
            compact: true,
            plugins: [terser()],
            sourcemap: true
        },
        plugins: createGlobalPlugins(true)
    },
];