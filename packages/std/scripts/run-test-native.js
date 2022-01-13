const {buildMatrix} = require('cmake-build');

buildMatrix({
    test: true
}).catch(_ => process.exit(1));