export let app = null;
export let analytics = null;

export function init() {
    const config = window["firebaseConfig"];
    if (!config) {
        console.warn("failed to initialize firebase, miss config object");
        return false;
    }
    app = firebase.initializeApp(config);
    analytics = firebase.analytics();

    return true;
}
