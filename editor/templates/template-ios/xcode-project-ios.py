import sys

# PBXPROJ: https://github.com/kronenthaler/mod-pbxproj/wiki
from pbxproj import XcodeProject, PBXGenericObject
from pbxproj.pbxextensions import FileOptions

print('Number of arguments:', len(sys.argv), 'arguments.')
print('Argument List:', str(sys.argv))

proj_ios_name = sys.argv[1]
application_id = sys.argv[2]


def disable_arc(project, file_path):
    for file in project.get_files_by_path(file_path):
        for build_file in project.get_build_files_for_file(file.get_id()):
            print("Disable ARC for: %s (%s)" % (file.name, build_file.get_id()))
            build_file.add_compiler_flags('-fno-objc-arc')


excludes = ["^build$", "^CMakeLists.txt$", "^.*\.md$"]
project = XcodeProject.load(f"{proj_ios_name}.xcodeproj/project.pbxproj")

project_target = project.get_target_by_name("template-ios")

sys_caps = PBXGenericObject()
sys_caps["com.apple.GameCenter.iOS"] = PBXGenericObject()
sys_caps["com.apple.GameCenter.iOS"]["enabled"] = 1
sys_caps["com.apple.InAppPurchase"] = PBXGenericObject()
sys_caps["com.apple.InAppPurchase"]["enabled"] = 1
project.objects[project.rootObject].attributes.TargetAttributes[project_target.get_id()][u'SystemCapabilities'] = sys_caps

# project.set_flags("DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym")

project_target.name = proj_ios_name
project_target.productName = proj_ios_name
project.set_flags("PRODUCT_BUNDLE_IDENTIFIER", application_id)
project.set_flags("IPHONEOS_DEPLOYMENT_TARGET", "12.0")

project.add_folder('../../src', parent=project.add_group("src"), excludes=excludes)
project.add_folder('../../../ecxx/src', parent=project.add_group("ecxx"), excludes=excludes)
project.add_folder('../../../ek-common/src', parent=project.add_group("ek-common"), excludes=excludes)
platforms_group = project.add_group("ek-platforms")
project.add_folder('../../../ek/platforms/apple', parent=platforms_group, excludes=excludes)
project.add_folder('../../../ek/platforms/ios', parent=platforms_group, excludes=excludes)
project.add_folder('../../../ek/src', parent=project.add_group("ek"), excludes=excludes)
project.add_folder('../../../scenex/src', parent=project.add_group("scenex"), excludes=excludes)

dependencies_group = project.add_group("dependencies")
project.add_file("../../deps/lodepng/lodepng.h", parent=dependencies_group)
project.add_file("../../deps/lodepng/lodepng.cpp", parent=dependencies_group)

disable_arc(project, '../../../ek/platforms/ios/ek/audiomini/SimpleAudioEngine_objc.mm')
disable_arc(project, '../../../ek/platforms/ios/ek/audiomini/CocosDenshion.mm')
disable_arc(project, '../../../ek/platforms/ios/ek/audiomini/CDOpenALSupport.mm')
disable_arc(project, '../../../ek/platforms/ios/ek/audiomini/CDAudioManager.mm')
disable_arc(project, '../../../ek/platforms/ios/EAGLView.mm')

project.add_header_search_paths([
    "$(inherited)",
    "../../src",
    "../../../ecxx/src",
    "../../../ek-common/src",
    "../../../ek/platforms",
    "../../../ek/src",
    "../../../scenex/src",
    "../../deps/lodepng"
])

# self.cpp_info.cxxflags.append("-fno-aligned-allocation")
frameworks = [
    "$(inherited)",
    "-framework UIKit",
    "-framework OpenGLES",
    "-framework QuartzCore",
    "-framework AudioToolbox",
    "-framework Foundation",
    "-framework OpenAL"
]

# file_options = FileOptions(weak=True)
file_options = FileOptions(weak=False, embed_framework=False)
project.add_file('System/Library/Frameworks/GameKit.framework', tree='SDKROOT', force=False, file_options=file_options)
project.add_file('System/Library/Frameworks/StoreKit.framework', tree='SDKROOT', force=False, file_options=file_options)

project.add_other_ldflags(" ".join(frameworks))
project.add_library_search_paths("$(inherited)")

project.add_other_cflags([
    "$(inherited)",
    "-DGLES_SILENCE_DEPRECATION"
])

project.add_file("assets", force=True)
project.add_file("../../GoogleService-Info.plist")

project.save()
