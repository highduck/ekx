import json
import os
import sys

# PBXPROJ: https://github.com/kronenthaler/mod-pbxproj/wiki
from pbxproj import XcodeProject, PBXGenericObject
from pbxproj.pbxextensions import FileOptions

config_file = open("ek-ios-build.json", "r")
config_data = json.loads(config_file.read())

print('Number of arguments: ' + str(len(sys.argv)) + ' arguments')
print('Argument List: ')
for arg in sys.argv:
    print(' * ' + arg)

proj_ios_name = sys.argv[1]
application_id = sys.argv[2]
sdk_root = sys.argv[3]

print('EKX ROOT: ( ' + sdk_root + " )")


def set_cpp_flags_for_files(project, files, flags):
    for file_path in files:
        rel_path = os.path.relpath(file_path, ".")
        for file in project.get_files_by_path(rel_path):
            for build_file in project.get_build_files_for_file(file.get_id()):
                build_file.add_compiler_flags(flags)


excludes = ["^build$", "^CMakeLists.txt$", "^.*\.md$", "^.*\.js$", "^.*\.glsl$"]
project = XcodeProject.load(f"{proj_ios_name}.xcodeproj/project.pbxproj")

project_target = project.get_target_by_name("template-ios")

# project.set_flags("DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym")

project_target.name = proj_ios_name
project_target.productName = proj_ios_name
project.set_flags("PRODUCT_BUNDLE_IDENTIFIER", application_id)
project.set_flags("IPHONEOS_DEPLOYMENT_TARGET", "12.0")

header_search_paths = ["$(inherited)"]
caps = []

def apply_module_settings(decl, group):
    if "assets" in decl:
        for src in decl["assets"]:
            project.add_file("assets", parent=group, force=True)

    if "cpp" in decl:
        for src in decl["cpp"]:
            project.add_folder(src, parent=group, excludes=excludes)
            header_search_paths.append(src)
    if "cpp_flags" in decl and "files" in decl["cpp_flags"]:
        set_cpp_flags_for_files(project, decl["cpp_flags"]["files"], decl["cpp_flags"]["flags"])
    if "xcode" in decl:
        if "frameworks" in decl["xcode"]:
            for fr in decl["xcode"]["frameworks"]:
                project.add_file("System/Library/Frameworks/" + fr + ".framework",
                                 tree='SDKROOT', force=False,
                                 file_options=FileOptions(weak=False, embed_framework=False))
        if "capabilities" in decl["xcode"]:
            for cap in decl["xcode"]["capabilities"]:
                caps.append(cap)



for module in config_data["modules"]:
    group = project.add_group(module["name"])
    apply_module_settings(module, group)
    if "ios" in module:
        apply_module_settings(module["ios"], group)

project.add_header_search_paths(header_search_paths)

if caps:
    sys_caps = PBXGenericObject()
    for cap in caps:
        sys_caps[cap] = PBXGenericObject()
        sys_caps[cap]["enabled"] = 1

    project.objects[project.rootObject].attributes.TargetAttributes[project_target.get_id()][
        'SystemCapabilities'] = sys_caps

# self.cpp_info.cxxflags.append("-fno-aligned-allocation")

project.add_other_ldflags("$(inherited) -Os -flto -fno-exceptions -fno-rtti")
project.add_library_search_paths("$(inherited)")

project.add_other_cflags([
    "$(inherited)",
    "-DGLES_SILENCE_DEPRECATION"
])

project.add_file("../../GoogleService-Info.plist")

project.save()
