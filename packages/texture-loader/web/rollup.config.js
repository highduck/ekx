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
        input: "./web/dist/module/index.js",
        output: {
            file: "./web/dist/emscripten/texture-loader.js",
            format: "iife",
            name: "EKXTextureLoader",
            compact: true,
            plugins: [terser()],
            sourcemap: true
        },
        plugins: createGlobalPlugins(true)
    }
];