/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "graphics",
        cpp: "src",
        apple: {
            cpp_flags: {
                files: [
                    "src/ek/ek_gfx.c"
                ],
                flags: "-x objective-c"
            },
        }
    });
    await project.importModule("@ekx/core");
    await project.importModule("@ekx/app");
    await project.importModule("@ekx/sokol");
}
