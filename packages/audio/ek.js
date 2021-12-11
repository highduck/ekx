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
                    "src/ek/audio/audio.cpp"
                ],
                flags: "-x objective-c++"
            },
        }
    });
    project.importModule("@ekx/auph", __dirname);
    project.importModule("@ekx/core", __dirname);
    project.importModule("@ekx/app", __dirname);
}

module.exports = setup;