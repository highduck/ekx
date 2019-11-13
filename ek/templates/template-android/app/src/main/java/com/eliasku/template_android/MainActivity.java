package com.eliasku.template_android;

import android.os.Bundle;

import ekapp.AppUtils;
import ekapp.EKActivity;

public class MainActivity extends EKActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        AppUtils.setDebugBuild(BuildConfig.DEBUG);
        super.onCreate(savedInstanceState);
    }
}