/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "std",
        cpp: "src",
        cpp_include: "include",
        web: {
            //cpp: "src-wasm",
            //cpp_lib: "--import-memory"
        }
    });
    await project.importModule("@ekx/stb");
    await project.importModule("@ekx/sokol");
}
