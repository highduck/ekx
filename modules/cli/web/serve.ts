import {serve as deno_serve} from "https://deno.land/std/http/mod.ts";
import {lookup} from "https://deno.land/x/media_types/mod.ts";

// import {createServer} from "http-server";
// const opener = require('opener');
//
// export function serve(dir: string) {
//     const server = createServer({
//         cache: -1,
//         root: dir
//     });
//     Deno.listen({ port: 1234 });
//     server.listen(1234, "0.0.0.0", function () {
//         opener("http://0.0.0.0:1234");
//     });
// }

export async function serve(dir: string) {
    const reqHandler = async (req: Request) => {
        const filePath = dir + new URL(req.url).pathname;
        let fileSize;
        try {
            fileSize = (await Deno.stat(filePath)).size;
        } catch (e) {
            if (e instanceof Deno.errors.NotFound) {
                return new Response(null, {status: 404});
            }
            return new Response(null, {status: 500});
        }
        const body = (await Deno.open(filePath)).readable;
        return new Response(body, {
            headers: {
                "content-length": fileSize.toString(),
                "content-type": lookup(filePath) || "application/octet-stream",
            },
        });
    };

    console.log("File server running on http://0.0.0.0:1234/index.html");
    await deno_serve(reqHandler, {hostname: "0.0.0.0", port: 1234});
// Start listening on port 8080 of localhost.
//     const server = Deno.listen({hostname: "0.0.0.0", port: 1234});
//     console.log("File server running on http://0.0.0.0:1234/index.html");
//
//     for await (const conn of server) {
//         handleHttp(conn).catch(console.error);
//     }
//
//     async function handleHttp(conn: Deno.Conn) {
//         const httpConn = Deno.serveHttp(conn);
//         for await (const requestEvent of httpConn) {
//             // Use the request pathname as filepath
//             const url = new URL(requestEvent.request.url);
//             const filepath = decodeURIComponent(url.pathname);
//
//             // Try opening the file
//             let file;
//             try {
//                 file = await Deno.open(path.join(dir, filepath), {read: true});
//             } catch(err) {
//                 console.error("Deno.open failed:", err);
//                 // If the file cannot be opened, return a "404 Not Found" response
//                 const notFoundResponse = new Response("404 Not Found", {status: 404});
//                 await requestEvent.respondWith(notFoundResponse);
//                 continue;
//             }
//
//             // Build a readable stream so the file doesn't have to be fully loaded into
//             // memory while we send it
//             const readableStream = file.readable;
//
//             // Build and send the response
//             const response = new Response(readableStream);
//             if (path.extname(filepath) === ".wasm") {
//                 response.headers.set("Content-Type", "application/wasm");
//             }
//             await requestEvent.respondWith(response);
//         }
//     }
}
