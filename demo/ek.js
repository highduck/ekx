const path = require("path");

module.exports = function (project) {
    project.modules.push({
        name: "picos",
        cpp: [path.join(__dirname, "src")],
        assets: [path.join(__dirname, "export/contents")]
    });

    const shortname = "ek-demo";
    const shortver = "6";

    project.name = shortname;
    project.title = "ekx";
    project.desc = "Engine Demo";
    project.binary_name = shortname;
    project.pwa_url = "/" + shortname;
    project.cmake_target = shortname;
    project.version_name = "1.0.6";
    project.version_code = shortver;
    project.build_number = shortver;
    project.orientation = "portrait";
    project.assets = {
        output: "export/contents/assets"
    };

    if (project.current_target === "android") {
        project.android = {
            application_id: "ilj.play.demo",
            package_id: "com.eliasku.iljdemo",
            admob_app_id: "ca-app-pub-3931267664278058~6275600638",
            game_services_id: "300613663654",
            keystore: {
                store_keystore: "release.keystore",
                store_password: "digiduckgames",
                key_alias: "eliasku",
                key_password: "digiduckgames"
            }

            /*
            debug {
            storeFile file('./debug2.keystore')
            storePassword 'android'
            keyAlias = 'androiddebugkey'
            keyPassword 'android'
             */
        };
    } else if (project.current_target === "ios") {
        project.ios = {
            application_id: "ilj.play.demo"
        };
    } else if (project.current_target === "web") {
        // TODO: update
        project.html = {
            google_analytics_property_id: "UA-3755607-9",
            background_color: "black",
            text_color: "#73b3cb",
            deploy_dir: "/Users/ilyak/ek/eliasku.github.io/picos/book",
            url: "https://eliasku.github.io/picos/book",
            image_url: "https://eliasku.github.io/picos/preview.png"
        };
    }
};