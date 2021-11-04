var LibLocalResource = {
    ek_fetch_open: function (pURL) {
        var getNext = function () {
            var next = table.length;
            for (var i = 1; i < next; ++i) {
                if (table[i] == null) {
                    return i;
                }
            }
            return next < 256 ? next : 0;
        };
        var table = window.EK_FETCH_OBJECTS;
        if (!table) {
            window.EK_FETCH_OBJECTS = table = [null];
        }
        var id = getNext();
        if (id) {
            table[id] = {url: UTF8ToString(pURL)};
        }
        return id;
    },
    ek_fetch_load: function (id) {
        var table = window.EK_FETCH_OBJECTS;
        if (!table) {
            return 1;
        }
        var obj = table[id];
        if (!obj || !obj.url) {
            return 2;
        }
        fetch(new Request(obj.url)).then(function (response) {
            return response.arrayBuffer();
        }).then(function (buffer) {
            var obj = table[id];
            if (obj) {
                obj.buffer = buffer;
                __ekfs_onComplete(id, 0, buffer.byteLength);
            }
        }).catch(function (reason) {
            var obj = table[id];
            if (obj) {
                obj.error = reason;
                __ekfs_onComplete(id, 1, 0);
            }
        });
        return 0;
    },
    ek_fetch_close: function (id) {
        var table = window.EK_FETCH_OBJECTS;
        if (table && table[id]) {
            table[id] = null;
            return 0;
        }
        return 1;
    },
    /**
     *
     * @param id {number}
     * @param pBuffer {number} - destination buffer pointer from C++, to set destination offset you just pass (buff + offset) pointer
     * @param toRead {number} - bytes to read from loaded buffer
     * @param offset {number} - source buffer offset from start
     */
    ek_fetch_read: function (id, pBuffer, toRead, offset) {
        var table = window.EK_FETCH_OBJECTS;
        if (!table) {
            return 0;
        }
        var obj = table[id];
        if (!obj) {
            return 0;
        }
        var buf = obj.buffer;
        if (!buf) {
            return 0;
        }
        {
            // DEBUG
            if (offset + toRead > buf.byteLength) {
                toRead = buf.byteLength - offset;
            }
        }
        var bytes = new Uint8Array(buf, offset, toRead);
        HEAPU8.set(bytes, pBuffer);
        return toRead;
    },
};

mergeInto(LibraryManager.library, LibLocalResource);