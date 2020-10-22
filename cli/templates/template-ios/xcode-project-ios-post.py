import sys

from pbxproj import XcodeProject, PBXShellScriptBuildPhase

proj_ios_name = sys.argv[1]

project = XcodeProject.load(f"{proj_ios_name}.xcodeproj/project.pbxproj")

# Fabric / Crashlytics
for target in project.objects.get_targets():
    shell = PBXShellScriptBuildPhase.create(
        "${PODS_ROOT}/FirebaseCrashlytics/run",
        input_paths=["$(SRCROOT)/$(BUILT_PRODUCTS_DIR)/$(INFOPLIST_PATH)"]
    )
    project.objects[shell.get_id()] = shell
    target.add_build_phase(shell)

project.save()
