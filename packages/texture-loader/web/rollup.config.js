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
            file: "./js/lib/lib-texture-loader.js",
            format: "es",
            compact: true,
            esModule: false,
            //plugins: [terser()],
            sourcemap: true
        },
        plugins: createGlobalPlugins(true)
    },
    {
        input: "./web/dist/module/index.js",
        output: {
            file: "./js/pre/texture-loader.js",
            format: "iife",
            name: "TextureLoader",
            compact: true,
            plugins: [terser()],
            sourcemap: true
        },
        plugins: createGlobalPlugins(true)
    },
];