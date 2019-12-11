import json
import os
import sys

# PBXPROJ: https://github.com/kronenthaler/mod-pbxproj/wiki
from pbxproj import XcodeProject, PBXGenericObject
from pbxproj.pbxextensions import FileOptions

config_file = open("ek-ios-build.json", "r")
ios_build_config = json.loads(config_file.read())
print('billing: ', ios_build_config["billing"])
print('dependencies: ', ios_build_config["dependencies"])

print('Number of arguments: ' + str(len(sys.argv)) + ' arguments')
print('Argument List: ')
for arg in sys.argv:
    print(' * ' + arg)

proj_ios_name = sys.argv[1]
application_id = sys.argv[2]
sdk_root = sys.argv[3]

print('EKX ROOT: ( ' + sdk_root + " )")


def disable_arc(project, file_path):
    rel_path = os.path.relpath(file_path, ".")
    print('Try Disable ARC for ' + rel_path)
    for file in project.get_files_by_path(rel_path):
        for build_file in project.get_build_files_for_file(file.get_id()):
            print("Disable ARC for: %s (%s)" % (file.name, build_file.get_id()))
            build_file.add_compiler_flags('-fno-objc-arc')


excludes = ["^build$", "^CMakeLists.txt$", "^.*\.md$"]
project = XcodeProject.load(f"{proj_ios_name}.xcodeproj/project.pbxproj")

project_target = project.get_target_by_name("template-ios")

sys_caps = PBXGenericObject()
sys_caps["com.apple.GameCenter"] = PBXGenericObject()
sys_caps["com.apple.GameCenter"]["enabled"] = 1
if ios_build_config["billing"]:
    sys_caps["com.apple.InAppPurchase"] = PBXGenericObject()
    sys_caps["com.apple.InAppPurchase"]["enabled"] = 1
    # sys_caps["com.apple.InAppPurchase"]["enabled"] = 1 if ios_build_config["billing"] else 0

project.objects[project.rootObject].attributes.TargetAttributes[project_target.get_id()][
    'SystemCapabilities'] = sys_caps

# project.set_flags("DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym")

project_target.name = proj_ios_name
project_target.productName = proj_ios_name
project.set_flags("PRODUCT_BUNDLE_IDENTIFIER", application_id)
project.set_flags("IPHONEOS_DEPLOYMENT_TARGET", "12.0")

project.add_folder('../../src', parent=project.add_group("src"), excludes=excludes)
project.add_folder(sdk_root + '/ecxx/src', parent=project.add_group("ecxx"), excludes=excludes)
project.add_folder(sdk_root + '/core', parent=project.add_group("ek-core"), excludes=excludes)
platforms_group = project.add_group("ek-platforms")
project.add_folder(sdk_root + '/ek/platforms/apple', parent=platforms_group, excludes=excludes)
project.add_folder(sdk_root + '/ek/platforms/ios', parent=platforms_group, excludes=excludes)
project.add_folder(sdk_root + '/ek/src', parent=project.add_group("ek"), excludes=excludes)
project.add_folder(sdk_root + '/scenex/src', parent=project.add_group("scenex"), excludes=excludes)

disable_arc(project, sdk_root + '/ek/platforms/ios/cocos-audio/SimpleAudioEngine_objc.mm')
disable_arc(project, sdk_root + '/ek/platforms/ios/cocos-audio/CocosDenshion.mm')
disable_arc(project, sdk_root + '/ek/platforms/ios/cocos-audio/CDOpenALSupport.mm')
disable_arc(project, sdk_root + '/ek/platforms/ios/cocos-audio/CDAudioManager.mm')

project.add_header_search_paths([
    "$(inherited)",
    "../../src",
    sdk_root + "/ecxx/src",
    sdk_root + "/core",
    sdk_root + "/ek/platforms/apple",
    sdk_root + "/ek/platforms/ios",
    sdk_root + "/ek/src",
    sdk_root + "/scenex/src"
])

# self.cpp_info.cxxflags.append("-fno-aligned-allocation")
frameworks = [
    "$(inherited)",
    "-framework UIKit",
    "-framework OpenGLES",
    "-framework QuartzCore",
    "-framework Foundation",
    # mini-audio
    "-framework OpenAL",
    "-framework AudioToolbox",
    "-framework AVFoundation",
]

# file_options = FileOptions(weak=True)
file_options = FileOptions(weak=False, embed_framework=False)
project.add_file('System/Library/Frameworks/GameKit.framework', tree='SDKROOT', force=False, file_options=file_options)
if ios_build_config["billing"]:
    project.add_file('System/Library/Frameworks/StoreKit.framework', tree='SDKROOT', force=False,
                     file_options=file_options)

project.add_other_ldflags(" ".join(frameworks))
project.add_other_ldflags("-Os -flto -fno-exceptions -fno-rtti")
project.add_library_search_paths("$(inherited)")

project.add_other_cflags([
    "$(inherited)",
    "-DGLES_SILENCE_DEPRECATION"
])

project.add_file("assets", force=True)
project.add_file("../../GoogleService-Info.plist")

project.save()
