mergeInto(LibraryManager.library, {
    firebase_js: function(cmd) {
        switch(cmd) {
            case 0: return window["firebase_js"]["init"]();
        }
        return false;
    }
});