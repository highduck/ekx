import {createServer} from "http-server";
const opener = require('opener');

export function serve(dir: string) {
    const server = createServer({
        cache: -1,
        root: dir
    });
    server.listen(1234, "0.0.0.0", function () {
        opener("http://0.0.0.0:1234");
    });
}