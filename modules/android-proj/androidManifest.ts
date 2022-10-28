import {XmlDocument} from "xmldoc";
import {writeTextFileSync} from "../utils/utils.js";

export interface UsesFeature {
    key: string;
    value: string;
    required?: boolean;
}

export class AndroidManifest {

    // root
    package = "";
    features: UsesFeature[] = [];
    permissions: string[] = [];

    _root: string[] = [];

    // application
    _application: string[] = [];

    // activity config;
    activityName = "com.eliasku.template_android.MainActivity";
    screenOrientation = "sensorPortrait";
    configChanges: string[] = [
        "keyboardHidden",
        "keyboard",
        "orientation",
        "screenSize",
        "layoutDirection",
        "locale",
        "uiMode",
        "screenLayout",
        "smallestScreenSize",
        "navigation"
    ];

    constructor() {
        this.features.push({
            key: "android:glEsVersion",
            value: "0x00020000",
            required: true
        });
        this.features.push({
            key: "android:name",
            value: "android.hardware.screen.landscape",
            required: false
        });
        this.features.push({
            key: "android:name",
            value: "android.hardware.screen.portrait",
            required: false
        });
    }

    save(filepath: string) {
        // <?xml version="1.0" encoding="utf-8"?>
        const doc = new XmlDocument(`<manifest package="${this.package}"></manifest>`);
        doc.attr["xmlns:android"] = "http://schemas.android.com/apk/res/android";
        doc.attr["xmlns:tools"] = "http://schemas.android.com/tools";

        for (const feature of this.features) {
            const el = new XmlDocument(`<uses-feature ${feature.key}="${feature.value}" />`);
            if (feature.required !== undefined) {
                el.attr["android:required"] = "" + feature.required;
            }
            doc.children.push(el);
        }

        for (const permission of this.permissions) {
            doc.children.push(new XmlDocument(
                `<uses-permission android:name="${permission}" />`
            ));
        }

        for (const node of this._root) {
            doc.children.push(new XmlDocument(node));
        }

        {
            // <application>
            const application = new XmlDocument(`<application/>`);
            doc.children.push(application);

            application.attr["android:name"] = "androidx.multidex.MultiDexApplication";
            application.attr["android:allowBackup"] = "false";
            application.attr["android:icon"] = "@mipmap/ic_launcher";
            application.attr["android:label"] = "@string/app_name";
            application.attr["android:supportsRtl"] = "true";
            application.attr["android:theme"] = "@style/AppTheme";

            {
                // <activity>
                const activity = new XmlDocument(`<activity/>`);
                application.children.push(activity);

                activity.attr["android:name"] = this.activityName;
                activity.attr["android:configChanges"] = this.configChanges.join("|");
                activity.attr["android:screenOrientation"] = this.screenOrientation;
                activity.attr["android:theme"] = "@style/AppTheme.Launch";
                activity.attr["android:launchMode"] = "singleTask";
                activity.attr["android:exported"] = "true";

                activity.children.push(new XmlDocument(`<intent-filter>
    <action android:name="android.intent.action.MAIN" />
    <category android:name="android.intent.category.LAUNCHER" />
</intent-filter>`))
            }

            for (const node of this._application) {
                application.children.push(new XmlDocument(node));
            }
        }

        // stringify and save file
        const text = doc.toString({
            preserveWhitespace: true,
            html: false
        });

        writeTextFileSync(filepath, text);
    }
}