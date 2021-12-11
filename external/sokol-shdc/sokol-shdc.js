#!/usr/bin/env node

const {spawnSync} = require("child_process");
const path = require("path");

const arguments = process.argv.slice(2);

console.info("sokol-shdc start on", process.platform);

const executables = {
    darwin: 'bin/osx/sokol-shdc',
    linux: 'bin/linux/sokol-shdc',
    win32: 'bin/win32/sokol-shdc.exe'
};

const executable = executables[process.platform];
const exe = path.join(__dirname, executable);
console.info(exe, ...arguments);
console.info("|cwd", process.cwd());

if (!executable) {
    console.error(`error: platform ${process.platform} is not supported`);
    process.exit(21);
}

const status = spawnSync(exe, arguments, {
    stdio: "inherit"
}).status;

process.exit(status);
