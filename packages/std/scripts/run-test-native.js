const {buildMatrix} = require('cmake-build');

buildMatrix({
    test: true,
    debug: true
}).catch(_ => process.exit(1));