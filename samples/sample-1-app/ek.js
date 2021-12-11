module.exports = function (project) {
    project.addModule({
        name: "sample-1-app",
        path: __dirname,
        cpp: "src"
    });

    project.title = "sample-1-app";
    project.desc = "sample-1-app";
    project.orientation = "portrait";
    project.importModule("@ekx/app");
};