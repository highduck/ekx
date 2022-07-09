export async function setup(project) {
    project.addModule({
        name: "sample-1-app",
        cpp: "src"
    });

    project.title = "sample-1-app";
    project.desc = "sample-1-app";
    project.orientation = "portrait";
    await project.importModule("@ekx/app");
}
