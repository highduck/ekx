/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "audio",
        path: __dirname,
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
    project.importModule("@ekx/auph", __dirname);
    project.importModule("@ekx/core", __dirname);
    project.importModule("@ekx/app", __dirname);
}

module.exports = setup;