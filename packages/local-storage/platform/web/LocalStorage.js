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
    }
});