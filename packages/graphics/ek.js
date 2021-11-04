/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "graphics",
        path: __dirname,
        cpp: "src",
        apple: {
            cpp_flags: {
                files: [
                    "src/ek/graphics/graphics.cpp"
                ],
                flags: "-x objective-c++"
            },
        }
    });
    project.importModule("@ekx/core", __dirname);
    project.importModule("@ekx/app", __dirname);
    project.importModule("@ekx/sokol", __dirname);
}

module.exports = setup;