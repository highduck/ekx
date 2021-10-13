/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "ekx-texture-loader",
        path: __dirname,
        cpp: "src",
        android: {
        },
        apple: {
        },
        ios: {
        },
        web: {
            js_pre: "web/dist/emscripten/texture-loader.js",
            js: "src/texture_loader"
        },
        windows: {
        },
        linux: {
        }
    });
    project.importModule("@ekx/sokol", __dirname);
}

module.exports = setup;