mergeInto(LibraryManager.library, {
    web_ls_set_f64: function (pKey, value) {
        var ls = window.localStorage;
        if (ls && pKey) {
            ls.setItem(UTF8ToString(pKey), value);
            return true;
        }
        return false;
    },
    web_ls_get_f64: function (pKey, pValue) {
        var ls = window.localStorage;
        if (ls && pKey) {
            var item = ls.getItem(UTF8ToString(pKey));
            if (item != null) {
                var value = parseFloat(item);
                if (value != null) {
                    if (pValue) {
                        HEAPF64[pValue >>> 3] = value;
                    }
                    return true;
                }
            }
        }
        return false;
    },
    /**
     *
     * @param pKey - NULL-terminated c-string
     * @param pValue - NULL-terminated c-string, if 0 - delete key,
     * @returns {boolean} - true if operation is completed, false if not supported or invalid arguments
     */
    web_ls_set: function (pKey, pValue) {
        var ls = window.localStorage;
        if (ls && pKey) {
            var key = UTF8ToString(pKey);
            if (pValue) {
                ls.setItem(key, UTF8ToString(pValue));
            } else {
                ls.removeItem(key);
            }
            return true;
        }
        return false;
    },
    /**
     * You can read or check key existence in user's LocalStorage
     *
     * @param pKey - CString or 0
     * @param pDest - Buffer to write the value (or 0 if you don't need read the value)
     * @param maxLength - Max buffer length
     * @returns {boolean} - true if key is valid and exists in storage, false otherwise
     */
    web_ls_get: function (pKey, pDest, maxLength) {
        var ls = window.localStorage;
        if (ls && pKey) {
            var value = ls.getItem(UTF8ToString(pKey));
            if (value != null) {
                if (pDest && maxLength > 0) {
                    stringToUTF8(value, pDest, maxLength);
                }
                return true;
            }
        }
        return false;
    },
    ekapp_setMouseCursor: function (cursor) {
        var PARENT = 0;
        var ARROW = 1;
        var BUTTON = 2;
        var HELP = 3;
        var map = [
            "auto", // 0
            "default", // 1
            "pointer", // 2
            "help" // 3
        ];
        cursor = cursor | 0;
        if (cursor >= 0 && cursor < map.length) {
            var gameview = document.getElementById("gameview");
            if (gameview) {
                gameview.style.cursor = map[cursor];
            }
        }
    },
    ekapp_getLang: function (dest, max) {
        var lang = window.navigator.language;
        if (lang != null && lang.length >= 2) {
            lang = lang.substr(0, 2);
            stringToUTF8(lang, dest, max);
            return dest;
        }
        return 0;
    },
    ekapp_openURL: function (pURL) {
        try {
            window.open(UTF8ToString(pURL), "_blank");
            return 0;
        } catch {
        }
        return 1;
    },

    ekapp_init__deps: ['$GL'],
    ekapp_init: function (flags) {
        var BUTTONS = [0, 2, 1, 2, 2];

        var TYPES = {
            "keydown": 14,
            "keyup": 15,
            "keypress": 16,

            "mousemove": 8,
            "mousedown": 9,
            "mouseup": 10,
            "wheel": 13,
            "touchstart": 5,
            "touchend": 7,
            "touchcancel": 7,
            "touchmove": 6,
        };
        var KEY_CODES = {
            "ArrowUp": 1,
            "ArrowDown": 2,
            "ArrowLeft": 3,
            "ArrowRight": 4,
            "Escape": 5,
            "Space": 6,
            "Enter": 7,
            "Backspace": 8,
            "A": 16,
            "C": 17,
            "V": 18,
            "X": 19,
            "Y": 20,
            "Z": 21,
            "W": 22,
            "S": 23,
            "D": 24,
        };

        var onKey = function (event) {
            var type = TYPES[event.type];
            if (type) {
                var code = KEY_CODES[event.code];
                if (code) {
                    if (__ekapp_onKey(type, code, 0)) {
                        event.preventDefault();
                    }
                }
            }
        };

        var wnd = window;
        wnd.addEventListener("keypress", onKey, true);
        wnd.addEventListener("keydown", onKey, true);
        wnd.addEventListener("keyup", onKey, true);

        var handleResize = function () {
            var dpr = window.devicePixelRatio;

            var div = document.getElementById("gamecontainer");
            var rc = div.getBoundingClientRect();
            var css_w = rc.width;
            var css_h = rc.height;

            // TODO: configurable min aspect (70/100)
            // TODO: landscape and different modes, native letterbox
            var w = css_w;
            var h = css_h;
            var offset_x = 0;
            var offset_y = 0;

            // TODO:
            //if (webKeepCanvasAspectRatio) {
            //var aspect = g_app.config.width / g_app.config.height;
            //if (aspect > 1.0) {
            //    if (w / aspect < h) {
            //        h = w / aspect;
            //    }
            //    offset_y = (css_h - h) / 2;
            //} else {
            //    if (h * aspect < w) {
            //        w = h * aspect;
            //    }
            //    offset_x = (css_w - w) / 2;
            //}
            //}

            var drawableWidth = (w * dpr) | 0;
            var drawableHeight = (h * dpr) | 0;

            __ekapp_onResize(dpr, w, h, drawableWidth, drawableHeight);

            var gameview = document.getElementById("gameview");
            if (gameview) {
                if (gameview.width !== drawableWidth ||
                    gameview.height !== drawableHeight) {

                    gameview.width = drawableWidth;
                    gameview.height = drawableHeight;

                    gameview.style.width = w + "px";
                    gameview.style.height = h + "px";
                }
                gameview.style.transform = "translateX(" + offset_x + "px) translateY(" + offset_y + "px)";
            }
        };

        // callback call after timeout after last call (if call again before timeout,
        // planned callback is cancelled and re-scheduled to be called after timeout)
        function throttle(callback, millisecondsLimit) {
            var timer = -1;
            return function () {
                if (timer >= 0) {
                    clearTimeout(timer);
                }
                timer = setTimeout(function () {
                    timer = -1;
                    callback();
                }, millisecondsLimit);
            }
        }

        wnd.addEventListener("resize", throttle(handleResize, 100), true);
        handleResize();

        /**
         *
         * @param e {TouchEvent}
         */
        var onTouch = function (e) {
            var type = TYPES[e.type];
            if (type) {
                // https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent/button
                var rect = e.target.getBoundingClientRect();
                var cancelDefault = false;
                for (var i = 0; i < e.changedTouches.length; ++i) {
                    const touch = e.changedTouches[i];
                    var id = touch.identifier + 1;
                    var x = touch.clientX - rect.left;
                    var y = touch.clientY - rect.top;
                    cancelDefault = cancelDefault || __ekapp_onTouch(type, id, x, y);
                }
                if (cancelDefault) {
                    e.preventDefault();
                }
            }
        };

        /**
         *
         * @param event {MouseEvent}
         */
        var onMouse = function (event) {
            var type = TYPES[event.type];
            // https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent/button
            var rect = event.target.getBoundingClientRect();
            var x = event.clientX - rect.left;
            var y = event.clientY - rect.top;
            if (type && __ekapp_onMouse(type, BUTTONS[event.button], x, y)) {
                event.preventDefault();
            }
        };

        /**
         *
         * @param event {WheelEvent}
         */
        var onWheel = function (event) {
            if (__ekapp_onWheel(event.deltaX, event.deltaY)) {
                event.preventDefault();
            }
        };

        var nonPassiveOpt = false;
        try {
            window.addEventListener("test", null, Object.defineProperty({}, 'passive', {
                get: function () {
                    nonPassiveOpt = {passive: false};
                }
            }));
        } catch (e) {
        }
        /** {CanvasElement} */
        var canvas = document.getElementById("gameview");
        canvas.addEventListener("mousedown", onMouse, nonPassiveOpt);
        canvas.addEventListener("mouseup", onMouse, nonPassiveOpt);
        canvas.addEventListener("mousemove", onMouse, nonPassiveOpt);
        canvas.addEventListener("wheel", onWheel, nonPassiveOpt);
        canvas.addEventListener("touchstart", onTouch, nonPassiveOpt);
        canvas.addEventListener("touchend", onTouch, nonPassiveOpt);
        canvas.addEventListener("touchmove", onTouch, nonPassiveOpt);
        canvas.addEventListener("touchcancel", onTouch, nonPassiveOpt);

        var webgl_list = ["webgl", "experimental-webgl"]; // 'webgl2'
        var webgl_attributes = {
            alpha: false,
            depth: !!(flags & 1),
            stencil: false,
            antialias: false
        };
        var gl = undefined;
        for (var i = 0; i < webgl_list.length; ++i) {
            gl = canvas.getContext(webgl_list[i], webgl_attributes);
            if (gl) {
                break;
            }
        }
        if (!gl) {
            console.error("Failed to create WebGL context");
            return false;
        }
        canvas.addEventListener("webglcontextlost", function (e) {
            alert("WebGL context lost. You will need to reload the page.");
            e.preventDefault();
        }, false);

        webgl_attributes.majorVersion = 1;
        // extensions required for sokol by default
        webgl_attributes.enableExtensionsByDefault = true;
        var handle = GL.registerContext(gl, webgl_attributes);
        if (!GL.makeContextCurrent(handle)) {
            console.error("Failed to set current WebGL context");
            return false;
        }

        // check visibility,
        // TODO: bind this with running loop and set after ready event
        var hidden, visibilityChange;
        if (typeof document.hidden !== "undefined") {
            // Opera 12.10 and Firefox 18 and later support
            hidden = "hidden";
            visibilityChange = "visibilitychange";
        } else if (typeof document.msHidden !== "undefined") {
            hidden = "msHidden";
            visibilityChange = "msvisibilitychange";
        } else if (typeof document.webkitHidden !== "undefined") {
            hidden = "webkitHidden";
            visibilityChange = "webkitvisibilitychange";
        }

        // Handle page visibility change
        var focused = true;
        var handleFocus = function(_) {
            var flags = 0;
            if(hidden !== undefined && !document[hidden]) {
                flags |= 1;
            }
            if(document.hasFocus()) {
                flags |= 2;
            }
            __ekapp_onFocus(flags);
        }

        if (typeof document.addEventListener === "undefined" || hidden === undefined) {
            console.warn("No Page Visibility API");
        } else {
            document.addEventListener(visibilityChange, handleFocus, false);
        }
        wnd.addEventListener("blur", handleFocus, false);
        wnd.addEventListener("focus", handleFocus, false);

        handleFocus();

        return true;
    },
    ekapp_run: function () {
        var loop = function () {
            requestAnimationFrame(loop);
            __ekapp_loop();
        };
        loop();
    },


    ////// Fetch
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

    ekapp_log: function (pStr) {
        console.log(UTF8ToString(pStr));
    }
});