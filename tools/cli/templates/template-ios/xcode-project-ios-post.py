import sys

from pbxproj import XcodeProject, PBXShellScriptBuildPhase

proj_ios_name = sys.argv[1]

mainTargetName = "app-ios"

project = XcodeProject.load(f"{mainTargetName}.xcodeproj/project.pbxproj")
app_target = project.get_target_by_name(mainTargetName)

# Fabric / Crashlytics
#for target in project.objects.get_targets():
shell = PBXShellScriptBuildPhase.create(
    "${PODS_ROOT}/FirebaseCrashlytics/run",
    input_paths=["$(SRCROOT)/$(BUILT_PRODUCTS_DIR)/$(INFOPLIST_PATH)"]
)
project.objects[shell.get_id()] = shell
app_target.add_build_phase(shell)

project.save()
