import {initializeApp} from "firebase/app";
import {getAnalytics} from "firebase/analytics";

// // TODO: Replace the following with your app's Firebase project configuration
// const firebaseConfig = {
//     //...
// };

const firebaseContext = {
    config: window.firebaseConfig,
    app: null,
    analytics: null
};

mergeInto(LibraryManager.library, {
    firebase_init: function() {
        if (typeof firebaseContext.config !== "undefined") {
            firebaseContext.app = initializeApp(firebaseContext.config);
            firebaseContext.analytics = getAnalytics(firebaseContext.app);
        }
    }
});