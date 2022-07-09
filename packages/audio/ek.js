/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "audio",
        cpp: "src",
        apple: {
            cpp_flags: {
                files: [
                    "src/ek_audio_auph.c"
                ],
                flags: "-x objective-c"
            },
        },
        android: {
            cpp_flags: {
                files: [
                    "src/ek_audio_auph.c"
                ],
                flags: "-xc++ -std=c++17"
            },
        }
    });
    await project.importModule("@ekx/auph");
    await project.importModule("@ekx/core");
    await project.importModule("@ekx/app");
}
