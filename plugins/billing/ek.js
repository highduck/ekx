/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "plugin-billing",
        path: __dirname,
        cpp: "src",
        android: {
            cpp: "android",
            android_java: "android/java",
            android_dependency: `implementation 'com.android.billingclient:billing:4.0.0'`
        },
        ios: {
            cpp: "ios",
            xcode_capability: "com.apple.InAppPurchase",
            xcode_framework: "StoreKit"
        },
        web: {
            cpp: "sim"
        },
        macos: {
            cpp: "sim"
        },
    });
}

module.exports = setup;