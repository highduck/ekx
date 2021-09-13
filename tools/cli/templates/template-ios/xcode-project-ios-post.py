import sys

from pbxproj import XcodeProject, PBXShellScriptBuildPhase

proj_ios_name = sys.argv[1]

mainTargetName = "app-ios"

project = XcodeProject.load(f"{mainTargetName}.xcodeproj/project.pbxproj")
app_target = project.get_target_by_name(mainTargetName)

# XCODE_POST_PROJECT

project.save()
