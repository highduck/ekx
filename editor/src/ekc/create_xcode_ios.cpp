#include "generate_project_common.hpp"
#include "project_config.h"

#include <ek/utility/strings.hpp>
#include <ek/fs/system.hpp>
#include <ek/fs/working_dir.hpp>
#include <plist/plist.hpp>

#include <string>
#include <ek/logger.hpp>

using std::string;
using std::vector;
using std::any;
using std::map;

using namespace ek;

// REQUIREMENTS:
//  - CocoaPods
//  - python3
//  - pip3 install pbxproj
// FILES:
// - GoogleService-Info.plist

namespace ekc {

void mod_plist(const project_config_t& config, const string& path = "src/Info.plist") {
    map<string, any> dict;
    ek::plist::read_plist(path.c_str(), dict);
    dict["CFBundleDisplayName"] = config.title;
    dict["CFBundleShortVersionString"] = config.version_name;
    dict["CFBundleVersion"] = config.version_code;
    dict["UIRequiresFullScreen"] = true;
    dict["UIStatusBarHidden"] = true;
    dict["UIStatusBarStyle"] = string{"UIStatusBarStyleDefault"};
    dict["UIViewControllerBasedStatusBarAppearance"] = false;

    if (config.orientation == "portrait") {
        dict["UISupportedInterfaceOrientations"] = vector<any>{
                {"UIInterfaceOrientationPortrait"}
        };
        dict["UISupportedInterfaceOrientations~ipad"] = vector<any>{
                {"UIInterfaceOrientationPortrait"},
                {"UIInterfaceOrientationPortraitUpsideDown"}
        };
    } else {
        dict["UISupportedInterfaceOrientations"] = vector<any>{
                {"UIInterfaceOrientationLandscapeLeft"},
                {"UIInterfaceOrientationLandscapeRight"}
        };
        dict["UISupportedInterfaceOrientations~ipad"] = vector<any>{
                {"UIInterfaceOrientationLandscapeLeft"},
                {"UIInterfaceOrientationLandscapeRight"}
        };
    }

    dict["GADIsAdManagerApp"] = true;

    ek::plist::write_plist_xml(path.c_str(), dict);
}

void create_xcode_ios(const project_config_t& config) {
    update_module_cache();

    const string platform_target = "ios";
    const string platform_proj_name = config.name + "-" + platform_target;
    const path_t dest_dir{"projects"};
    const path_t dest_path = dest_dir / platform_proj_name;

    if (is_dir(dest_path)) {
        EK_INFO << "Remove old project: " << dest_path;
        remove_dir_rec(dest_path);
        assert(!is_dir(dest_path));
    }

    copy_tree(config.path_ekc / ("templates/template-" + platform_target), dest_path);

    path_t base_path{"../.."};
    working_dir_t::with(dest_path, [&]() {
        EK_INFO << "Rename project";
        rename("template-ios.xcodeproj",
               (platform_proj_name + ".xcodeproj").c_str());

        copy_tree(base_path / config.assets.output, path_t{"assets"});
        copy_tree(base_path / "generated/ios/AppIcon.appiconset",
                  path_t{"src/Assets.xcassets/AppIcon.appiconset"});

        const path_t src_launch_logo_path{"generated/ios/AppIcon.appiconset"};
        const path_t dest_launch_logo_path = base_path / "src/Assets.xcassets/LaunchLogo.imageset";
        // launch logo
        copy_file(src_launch_logo_path / "iphone_40.png",
                  dest_launch_logo_path / "iphone_40.png");
        copy_file(src_launch_logo_path / "iphone_80.png",
                  dest_launch_logo_path / "iphone_80.png");
        copy_file(src_launch_logo_path / "iphone_120.png",
                  dest_launch_logo_path / "iphone_120.png");

        mod_plist(config, "src/Info.plist");

        /// PRE MOD PROJECT
        execute("python3 xcode-project-ios.py " + platform_proj_name + " " + config.ios.application_id);

        EK_INFO << "Prepare PodFile";
        replace_in_file(path_t{"Podfile"}, {
                {"template-ios", platform_proj_name}
        });

        EK_INFO << "Install Pods";
        execute("pod install");

        /// POST MOD PROJECT
        execute("python3 xcode-project-ios-post.py " + platform_proj_name + " " + config.ios.application_id);
    });

}

}