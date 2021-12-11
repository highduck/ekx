const path = require("path");

module.exports = function (project) {
    project.addModule({
        name: "rnd-test",
        path: __dirname,
        cpp: "src"
    });

    project.title = "rnd quality test";
    project.desc = "rnd quality test";
    project.orientation = "portrait";

    const secretPath = '../../keys/demo';

    project.android.application_id = "ilj.play.demo";
    project.android.package_id = "com.eliasku.iljdemo";
    project.android.admob_app_id = "ca-app-pub-3931267664278058~6275600638";
    project.android.game_services_id = "300613663654";
    project.android.googleServicesConfigDir = secretPath;
    project.android.signingConfigPath = path.join(secretPath, "google-play-signing.json");
    project.android.serviceAccountKey = '../keys/_fastlane/api-5415425812860184091-52535-0303d398e1e6.json';

    project.ios.application_id = "ilj.play.demo";
    project.ios.admob_app_id = "ca-app-pub-3931267664278058~8636974437";
    project.ios.googleServicesConfigDir = secretPath;
    project.ios.appStoreCredentials = '../../keys/_fastlane/appstore.json';

    project.web.firebaseToken = "../../keys/_firebase/token.txt";
    project.web.applications = [
        {
            platform: "play",
            url: "https://play.google.com/store/apps/details?id=ilj.play.demo",
            id: "ilj.play.demo"
        }
    ];

    project.web.firebaseAutoSetup = true;
    project.web.background_color = "black";
    project.web.text_color = "#73b3cb";
    project.web.og = {
        url: "https://play-ilj.web.app",
        image: "https://play-ilj.web.app/icons/icon192.png"
    };

    project.importModule("@ekx/graphics");
};