import {path} from "../../deps.ts";

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
// Start listening on port 8080 of localhost.
    const server = Deno.listen({hostname: "0.0.0.0", port: 1234});
    console.log("File server running on http://0.0.0.0:1234/index.html");

    for await (const conn of server) {
        handleHttp(conn).catch(console.error);
    }

    async function handleHttp(conn: Deno.Conn) {
        const httpConn = Deno.serveHttp(conn);
        for await (const requestEvent of httpConn) {
            // Use the request pathname as filepath
            const url = new URL(requestEvent.request.url);
            const filepath = decodeURIComponent(url.pathname);

            // Try opening the file
            let file;
            try {
                file = await Deno.open(path.join(dir, filepath), {read: true});
            } catch(err) {
                console.error("Deno.open failed:", err);
                // If the file cannot be opened, return a "404 Not Found" response
                const notFoundResponse = new Response("404 Not Found", {status: 404});
                await requestEvent.respondWith(notFoundResponse);
                continue;
            }

            // Build a readable stream so the file doesn't have to be fully loaded into
            // memory while we send it
            const readableStream = file.readable;

            // Build and send the response
            const response = new Response(readableStream);
            if (path.extname(filepath) === ".wasm") {
                response.headers.set("Content-Type", "application/wasm");
            }
            await requestEvent.respondWith(response);
        }
    }
}
