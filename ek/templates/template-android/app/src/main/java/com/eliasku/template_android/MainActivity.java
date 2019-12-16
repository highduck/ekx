package com.eliasku.template_android;

import android.os.Bundle;

public class MainActivity extends ek.EkActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        ek.AppUtils.setDebugBuild(BuildConfig.DEBUG);
        super.onCreate(savedInstanceState);
    }
}