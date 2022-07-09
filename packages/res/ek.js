/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "@ekx/res",
        cpp: "src",
        cpp_include: "include",
        web: {
            js: "src"
        }
    });
    await project.importModule("@ekx/app");
}
