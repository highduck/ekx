mergeInto(LibraryManager.library, {
    firebase_js: function(cmd) {
        switch(cmd) {
            case 0: return window["firebase_js"]["init"]();
            case 1: return window["firebase_js"]["login"]();
            case 2: return window["firebase_js"]["logout"]();
            case 3: return window["firebase_js"]["delete_account"]();
        }
        return false;
    }
});