mergeInto(LibraryManager.library, {
    ek_app_js_set_mouse_cursor: function (cursor) {
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

    ek_app_js_init__deps: ['$GL'],
    ek_app_js_init: function (flags) {
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
            "KeyA": 16,
            "KeyC": 17,
            "KeyV": 18,
            "KeyX": 19,
            "KeyY": 20,
            "KeyZ": 21,
            "KeyW": 22,
            "KeyS": 23,
            "KeyD": 24,
        };

        var onKey = function (event) {
            var type = TYPES[event.type];
            if (type) {
                var code = KEY_CODES[event.code];
                if (code) {
                    if (_ek_app_js__on_key(type, code, 0)) {
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
            //var aspect = ek_app.config.width / ek_app.config.height;
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

            _ek_app_js__on_resize(drawableWidth, drawableHeight, dpr);

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
                    cancelDefault = cancelDefault || _ek_app_js__on_touch(type, id, x, y);
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
            if (type && _ek_app_js__on_mouse(type, BUTTONS[event.button], x, y)) {
                event.preventDefault();
            }
        };

        /**
         *
         * @param event {WheelEvent}
         */
        var onWheel = function (event) {
            if (_ek_app_js__on_wheel(event.deltaX, event.deltaY)) {
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
            _ek_app_js__on_focus(flags);
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
    ek_app_js_run: function () {
        var loop = function () {
            requestAnimationFrame(loop);
            _ek_app_js__loop();
        };
        loop();
    },
    ek_app_js_close: function () {
        window.close();
    },
    ek_app_js_lang: function(dest, maxLength) {
        var lang = window.navigator.language;
        if (lang) {
            stringToUTF8(lang, dest, maxLength);
        }
        else {
            HEAPU8[dest] = 0;
        }
    },
    ek_app_js_navigate: function (pURL) {
        try {
            window.open(UTF8ToString(pURL), "_blank");
            return 0;
        } catch {
        }
        return 1;
    },
    ek_app_js_share: function(pContent) {
        if (navigator.share) {
            navigator.share({
                // title: "",
                // url: "",
                text: UTF8ToString(pContent)
            }).then(() => {
                //console.log('Thanks for sharing!');
            }).catch(console.error);
            return 0;
        } else {
            // not supported
            return 1;
        }
    }
});