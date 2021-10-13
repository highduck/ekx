// import {terser} from 'rollup-plugin-terser';
import {nodeResolve} from "@rollup/plugin-node-resolve"
export default {
    input: "./web/firebase.js",
    output: {
        // support core api for emscripten implementation
        file: "./web/dist/firebase.js",
        format: "iife",
        compact: true,
        //   plugins: [terser()],
        sourcemap: true
    },
    plugins: [
        nodeResolve()
    ]
};