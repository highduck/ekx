const path = require("path");

module.exports = (ctx) => {
    ctx.addModule({
        name: "plugin-billing",
        cpp: [path.join(__dirname, "src")],
        android: {
            cpp: [path.join(__dirname, "android")],
            java: [path.join(__dirname, "android/java")]
        },
        ios: {
            cpp: [path.join(__dirname, "ios")],
            xcode: {
                capabilities: ["com.apple.InAppPurchase"],
                frameworks: ["StoreKit"]
            }
        },
        web: {
            cpp: [path.join(__dirname, "sim")]
        },
        macos: {
            cpp: [path.join(__dirname, "sim")]
        },
    });

    ctx.build.android.dependencies.push(`implementation 'com.android.billingclient:billing:3.0.0'`);
};