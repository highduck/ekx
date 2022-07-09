/**
 *
 * @param {Project} project
 */
export function setup(project) {
    project.addModule({
        name: "box2d",
        cpp: ["src", "include"]
    });
}
