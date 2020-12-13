const path = require('path');

module.exports = (project) => {
    project.addModule({
        name: "appbox",
        cpp: [path.join(__dirname, "src")]
    });

    project.includeProject(path.join(project.path.EKX_ROOT, "plugins/admob"));
    project.includeProject(path.join(project.path.EKX_ROOT, "plugins/billing"));
    project.includeProject(path.join(project.path.EKX_ROOT, "plugins/game-services"));
};