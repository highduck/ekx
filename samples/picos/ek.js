module.exports = class File {
    constructor(project) {
        const shortname = "picos";
        const shortver = "2";

        project.name = shortname;
        project.title = shortname;
        project.desc = shortname;
        project.binary_name = shortname;
        project.pwa_url = "/" + shortname;
        project.cmake_target = shortname;
        project.version_name = "0.0.2";
        project.version_code = shortver;
        project.build_number = shortver;
        project.orientation = "landscape";
        project.assets = {
            output: "build/assets"
        };

        if (project.current_target === "web") {
            project.html = {
                google_analytics_property_id: "UA-3755607-9",
                background_color: "black",
                text_color: "#73b3cb",
                deploy_dir: "/Users/ilyak/ek/eliasku.github.io/picos/book",
                url: "https://eliasku.github.io/picos/book",
                image_url: "https://eliasku.github.io/picos/preview.png"
            };
        }
        if (project.current_target === "android") {

            project.android = {
                application_id: "com.eliasku.odd_color_sense_vision_test_challenge",
                package_id: "com.eliasku.colorsense",
                admob_app_id: "ca-app-pub-3931267664278058~3023709586",
                game_services_id: "233407531247",
                keystore: {
                    store_keystore: "quack.keystore",
                    store_password: "quackquack",
                    key_alias: "key0",
                    key_password: "quackquack"
                }
            };
        }
        if (project.current_target === "ios") {

            project.ios = {
                application_id: "com.eliasku.oddcolorsensevisiontestchallenge"
            };
        }
    }
};