// import {terser} from 'rollup-plugin-terser';
import {nodeResolve} from "@rollup/plugin-node-resolve"
export default [{
    input: "./web/firebase.js",
    output: {
        file: "./web/dist/firebase.js",
        format: "iife",
        compact: true,
        name: "firebase_js",
        //   plugins: [terser()],
        sourcemap: true
    },
    plugins: [
        nodeResolve()
    ]
},
    {
        input: "./web/firebase_c.js",
        output: {
            file: "./web/lib/firebase_c.js",
            format: "iife",
            compact: true,
            sourcemap: true
        },
        plugins: [
            nodeResolve()
        ]
    }
];