package ek.billing;

import androidx.annotation.Keep;

import ek.EkActivity;
import ek.EkPluginManager;

public class BillingBridge {

    static BillingPlugin plugin;

    @Keep
    public static void initialize(String developerKey) {
        plugin = new GooglePlayBilling(EkActivity.getInstance(), developerKey);
        EkPluginManager.instance.extensions.add(plugin);
    }

    @Keep
    public static String getName() {
        return plugin.getName();
    }

    @Keep
    public static void getPurchases() {
        plugin.getPurchases();
    }

    @Keep
    public static void getDetails(String[] skus) {
        plugin.getDetails(skus);
    }

    @Keep
    public static void purchase(String sku, String payload) {
        plugin.purchase(sku, payload);
    }

    @Keep
    public static void consume(String token) {
        plugin.consume(token);
    }

    // callbacks
    native static void nativePurchase(String productID, String purchaseToken, int purchaseState, String payload, String signature);

    native static void nativeDetails(String sku, String price, String currencyCode);
}
