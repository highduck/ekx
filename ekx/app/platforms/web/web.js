mergeInto(LibraryManager.library, {
    web_prefs_set_number: function (key, n) {
        window.localStorage.setItem(UTF8ToString(key), n);
    },
    web_prefs_get_number: function (key, def) {
        var item = window.localStorage.getItem(UTF8ToString(key));
        if (item) {
            var val = parseFloat(item);
            if (val != null) {
                return val;
            }
        }
        return def;
    },
    web_prefs_set_string: function (key, str) {
        window.localStorage.setItem(UTF8ToString(key), UTF8ToString(str));
    },
    web_prefs_get_string: function (key) {
        var item = window.localStorage.getItem(UTF8ToString(key));
        if (item != null) {
            var lengthBytes = lengthBytesUTF8(item) + 1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(item, stringOnWasmHeap, lengthBytes);
            return stringOnWasmHeap;
        }
        return 0;
    },
    web_set_mouse_cursor: function (cursor) {
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
    web_update_gameview_size: function (width, height, dpr, offsetX, offsetY) {
        var gameview = document.getElementById("gameview");
        if (gameview) {
            var drawableWidth = (width * dpr) | 0;
            var drawableHeight = (height * dpr) | 0;
            if (gameview.width !== drawableWidth ||
                gameview.height !== drawableHeight) {
                gameview.width = (width * dpr) | 0;
                gameview.height = (height * dpr) | 0;
            }
            gameview.style.width = width + "px";
            gameview.style.height = height + "px";
            gameview.style.transform = "translateX(" + offsetX + "px) translateY(" + offsetY + "px)";
        }
    },
    web_get_lang: function (dest, max) {
        var lang = window.navigator.language;
        if(lang != null && lang.length >= 2) {
            lang = lang.substr(0, 2);
            stringToUTF8(lang, dest, max);
            return dest;
        }
        return 0;
    },
    web_vibrate: function (duration) {
        if (typeof window.navigator.vibrate === "function") {
            window.navigator.vibrate(duration);
        }
    }
});