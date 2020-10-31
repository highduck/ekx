const path = require("path");

module.exports = (ctx) => {
    ctx.modules.push({
        name: "plugin-billing",
        cpp: [path.join(__dirname, "src")],
        android: {
            cpp: [path.join(__dirname, "android")],
            java: [path.join(__dirname, "android/java")],
            aidl: [path.join(__dirname, "android/aidl")]
        },
        ios: {
            cpp: [path.join(__dirname, "ios")],
            xcode: {
                capabilities: ["com.apple.InAppPurchase"],
                frameworks: ["StoreKit"]
            }
        }
    });

    ctx.build.android.add_manifest.push(`
    <uses-permission android:name="com.android.vending.BILLING" />`);
};