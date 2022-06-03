const {buildMatrix} = require('cmake-build');

buildMatrix({
    test: true,
    // target: "all",
    debug: true
}).catch(_ => process.exit(1));