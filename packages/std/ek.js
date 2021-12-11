/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "std",
        path: __dirname,
        cpp: "src",
        cpp_include: "include",
        web: {
            //cpp: "src-wasm",
            cpp_lib: "--import-memory"
        }
    });
    project.importModule("@ekx/stb", __dirname);
    project.importModule("@ekx/sokol", __dirname);
}

module.exports = setup;