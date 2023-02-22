var cacheName = "{{name}}_{{version_name}}_{{version_code}}";
var contentToCache = [];

self.addEventListener('install', function (event) {
    event.waitUntil(
        caches.open(cacheName).then(function (cache) {
            return cache.addAll(contentToCache);
        })
    );
});

self.addEventListener('activate', function (event) {
    event.waitUntil(
        caches.keys().then(function (keyList) {
            var tasks = [];
            for (var i = 0; i < keyList.length; ++i) {
                var key = keyList[i];
                if (key !== cacheName) {
                    tasks.push(caches.delete(key));
                }
            }
            return Promise.all(tasks);
        })
    );
});

self.addEventListener('fetch', function (event) {
    var request = event.request;
    if (request.method !== 'GET') return;
    if (request.url.indexOf(self.location.origin) !== 0) return;
    if (request.url.indexOf("/manifest.json") >= 0) return;
    if (request.url.indexOf("/sw.js") >= 0) return;

    // TODO:
    // - we able to do partial responses from cache
    // - requires for streaming webassembly module instantiation, could be ignored for sync instantiation
    if (request.headers.has('range')) return;
    //if (request.url.indexOf(".wasm") >= 0) return;

    event.respondWith(caches.match(request).then(function (response) {
        if (response) {
            return response;
        }
        request.cache = "no-cache";
        return fetch(request).then(function (resp) {
            var respClone = resp.clone();
            caches.open(cacheName).then(function (cache) {
                cache.put(request, respClone);
            });
            return resp;
        });
    }));
});

self.addEventListener('message', function(event) {
    if (event.data === 'SKIP_WAITING') {
        self.skipWaiting();
    }
});