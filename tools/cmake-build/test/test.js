'use strict';

const {convertPackageName} = require('../dist/index.js');

function expectEquals(a, b) {
    console.assert(a === b, `${a} should be ${b}`);
}

expectEquals(convertPackageName("@ekx/library-name"), "ekx::library-name");
expectEquals(convertPackageName("simple-library-name"), "simple-library-name");