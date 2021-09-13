mergeInto(LibraryManager.library, {
    web_ls_set_f64: function (pKey, value) {
        var ls = window.localStorage;
        if(ls && pKey) {
            ls.setItem(UTF8ToString(pKey), value);
            return true;
        }
        return false;
    },
    web_ls_get_f64: function (pKey, pValue) {
        var ls = window.localStorage;
        if(ls && pKey) {
            var item = ls.getItem(UTF8ToString(key));
            if(item != null) {
                var value = parseFloat(item);
                if (value != null) {
                    if(pValue) {
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
        if(ls && pKey) {
            var key = UTF8ToString(pKey);
            if(pValue) {
                ls.setItem(key, UTF8ToString(pValue));
            }
            else {
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
        if(ls && pKey) {
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
    ekapp_vibrate: function (duration) {
        var vibrate = window.navigator.vibrate;
        if(vibrate) {
            vibrate(duration);
            return 0;
        }
        return 1;
    },
    ekapp_openURL: function(url) {
        try {
            window.open(UTF8ToString(url), "_blank");
            return 0;
        }
        catch {}
        return 1;
    }
});