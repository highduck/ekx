import {Project} from "../../modules/cli/project.ts";

export function setup(project: Project) {
    project.addModule({
        name: "plugin-billing",
        cpp: "src",
        android: {
            android_java: "java",
            android_dependency: `implementation 'com.android.billingclient:billing:5.0.0'`
        },
        ios: {
            xcode_capability: "com.apple.InAppPurchase",
            xcode_framework: "StoreKit",
            cpp_flags: {
                files: [
                    "src/billing.cpp"
                ],
                flags: "-x objective-c++"
            }
        }
    });
}
