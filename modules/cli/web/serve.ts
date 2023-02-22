import {createServer, IncomingMessage, OutgoingHttpHeaders, ServerResponse} from "http";
import {readFile} from "fs";
import {extname, join} from "path";
import {logger} from "../logger.js";

const defaultHttpHeaders: OutgoingHttpHeaders = {
    "content-type": "application/octet-stream",
    "cache-control": "no-cache"
};

const MIME_TYPES_1: Record<string, OutgoingHttpHeaders> = {
    ".json": {
        "content-type": "application/json"
    },
    ".html": {
        "content-type": "text/html;charset=utf-8",
        "cache-control": "no-cache",
    },
    ".js": {
        "content-type": "text/javascript",
        "cache-control": "no-cache"
    },
    ".ttf": {
        "content-type": "font/ttf",
        "cache-control": "max-age=86400"
    },
    ".wasm": {
        "content-type": "application/wasm",
        "cache-control": "no-cache"
    },
};

export async function serve(dir: string) {

    const serveStatic = (file: string, res: ServerResponse, mime: OutgoingHttpHeaders) =>
        readFile(
            join(dir, file),
            (err, data) => {
                res.writeHead(err ? 404 : 200, mime);
                res.end(data);
            }
        );

    const error = (req: IncomingMessage, res: ServerResponse) => {
        res.writeHead(500);
        res.end();
    }

    const server = createServer((req: IncomingMessage, res: ServerResponse) => {
        if (req.url && req.method === "GET") {
            if (req.url === "/") {
                req.url = "/index.html";
            }
            let filepath = req.url;
            if(filepath.indexOf("?") >= 0) filepath = filepath.slice(0, filepath.indexOf("?"));
            const ext = extname(filepath);
            const headers = MIME_TYPES_1[ext] ?? defaultHttpHeaders;
            serveStatic(filepath, res, headers);
        } else {
            error(req, res);
        }
    });
    return new Promise((resolve) => {
        server.on("close", resolve);
        server.listen(+(process.env.PORT as string) || 8080);
        logger.info("Start server on http://localhost:8080");
    });
}
