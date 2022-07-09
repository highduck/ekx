/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "quickjs",
        cpp: "src",
        cpp_define: ['CONFIG_VERSION="1"', "_GNU_SOURCE"],
    });
}
