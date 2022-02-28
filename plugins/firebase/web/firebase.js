export let app = null;
export let analytics = null;
export let auth = null;
export let authUI = null;

export function init() {
    const config = window["firebaseConfig"];
    if (!config) {
        console.warn("failed to initialize firebase, miss config object");
        return false;
    }
    app = firebase.initializeApp(config);
    analytics = firebase.analytics();
    auth = firebase.auth();

    authUI = new firebaseui.auth.AuthUI(auth);
    if (authUI.isPendingRedirect()) {
        startAuthUI();
    }

    return true;
}

function startAuthUI() {
    // FirebaseUI config.
    var uiConfig = {
        signInSuccessUrl: '.',
        signInOptions: [
            // Leave the lines as is for the providers you want to offer your users.
            firebase.auth.GoogleAuthProvider.PROVIDER_ID,
            firebase.auth.FacebookAuthProvider.PROVIDER_ID,
            firebase.auth.TwitterAuthProvider.PROVIDER_ID,
            firebase.auth.GithubAuthProvider.PROVIDER_ID,
            firebase.auth.EmailAuthProvider.PROVIDER_ID,
            firebase.auth.PhoneAuthProvider.PROVIDER_ID,
            firebaseui.auth.AnonymousAuthProvider.PROVIDER_ID
        ],

        // tosUrl and privacyPolicyUrl accept either url string or a callback
        // function.
        // Terms of service url/callback.
        tosUrl: '<your-tos-url>',

        // Privacy policy url/callback.
        privacyPolicyUrl: function () {
            window.location.assign('<your-privacy-policy-url>');
        }
    };

    // The start method will wait until the DOM is loaded.
    authUI.start('#firebaseui-auth-container', uiConfig);
}

export function login() {
    startAuthUI();
}

export function logout() {
    firebase.auth().signOut();
}

export function delete_account() {
    firebase.auth().currentUser.delete().catch(function(error) {
        if (error.code === "auth/requires-recent-login") {
            // The user's credential is too old. She needs to sign in again.
            firebase.auth().signOut().then(function() {
                // The timeout allows the message to be displayed after the UI has
                // changed to the signed out state.
                setTimeout(function() {
                    alert('Please sign in again to delete your account.');
                }, 1);
            });
        }
    });
}