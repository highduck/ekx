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
            android_java: "java",
            android_dependency: `implementation 'com.android.billingclient:billing:4.1.0'`
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

module.exports = setup;