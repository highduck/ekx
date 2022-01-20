package ek.billing;

import android.app.Activity;

import ek.EkPlugin;

public abstract class BillingPlugin extends EkPlugin {

    // (arbitrary) request code for the purchase flow
    protected static final int RC_REQUEST = 10111;

    final Activity activity;
    final String developerKey;

    BillingPlugin(Activity activity, String developerKey) {
        this.developerKey = developerKey;
        this.activity = activity;
    }

    public abstract void getPurchases();

    public abstract void getDetails(String[] skus);

    public abstract void purchase(String sku, String payload);

    public abstract void consume(String token);

    public abstract String get_tag();
}
