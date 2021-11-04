/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "@ekx/res",
        path: __dirname,
        cpp: "src",
        web: {
            js: "platform/web"
        },
        apple: {
            cpp_flags: {
                files: [
                    "src/ek/LocalResource.cpp"
                ],
                flags: "-x objective-c++"
            },
        }
    });
    project.importModule("@ekx/app", __dirname);
}

module.exports = setup;