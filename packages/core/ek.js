/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "core",
        cpp: "src",
        cpp_include: "include"
    });
    await project.importModule("@ekx/std");
}
