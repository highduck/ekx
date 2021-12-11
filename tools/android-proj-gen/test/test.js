const {AndroidProjGen} = require("../dist/index.js");

const proj = new AndroidProjGen();
proj.save("build/test-project");
