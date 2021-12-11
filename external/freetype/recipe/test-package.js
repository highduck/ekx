const {buildMatrix} = require('cmake-build');

buildMatrix({
    cmakePath: "test",
    test: true
}).catch(_ => process.exit(1));