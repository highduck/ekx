package ek.billing;

import android.app.Activity;
import android.util.Log;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClient.BillingResponseCode;
import com.android.billingclient.api.BillingClient.SkuType;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.Purchase.PurchaseState;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsParams;
import com.android.billingclient.api.SkuDetailsResponseListener;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import ek.EkActivity;

@Keep
public class GooglePlayBilling extends BillingPlugin implements PurchasesUpdatedListener {

    final private static String TAG = "GooglePlayBilling";

    String pendingPurchase;
    final private BillingClient billingClient;
    private final Map<String, SkuDetails> products = new HashMap<>();
    boolean autoFinishTransactions = false;

    public GooglePlayBilling(Activity activity, String developerKey) {
        super(activity, developerKey);

        billingClient = BillingClient.newBuilder(activity).setListener(this).enablePendingPurchases().build();
        billingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(@NonNull BillingResult billingResult) {
                if (billingResult.getResponseCode() == BillingResponseCode.OK) {
                    Log.v(TAG, "Setup finished");
                    // NOTE: we will not query purchases here. This is done
                    // when the extension listener is set
                } else {
                    Log.wtf(TAG, "Setup error: " + billingResult.getDebugMessage());
                }
            }

            @Override
            public void onBillingServiceDisconnected() {
                Log.v(TAG, "Service disconnected");
            }
        });
    }

    @Override
    public String getName() {
        return "GooglePlay";
    }

    @Override
    public void getPurchases() {
        Log.d(TAG, "processPendingConsumables()");
        queryPurchases(SkuType.INAPP);
        queryPurchases(SkuType.SUBS);
    }

    @Override
    public void getDetails(String[] skus) {
        final ArrayList<String> skuList = new ArrayList<>();
        Collections.addAll(skuList, skus);

        querySkuDetailsAsync(skuList, (billingResult, skuDetails) -> {
            if (billingResult.getResponseCode() == BillingResponseCode.OK) {
                if (skuDetails != null) {
                    EkActivity.runGLThread(() -> {
                        for (SkuDetails sd : skuDetails) {
                            BillingBridge.nativeDetails(sd.getSku(), sd.getPrice(), sd.getPriceCurrencyCode());
                        }
                    });
                }
            } else {
                Log.e(TAG, "Unable to list products: " + billingResult.getDebugMessage());
            }
        });
    }

    @Override
    public void purchase(String sku, String developerPayload) {
        // TODO: remove developerPayload
        pendingPurchase = sku;
        SkuDetails skuDetails = this.products.get(sku);
        if (skuDetails != null) {
            buyProduct(skuDetails);
        } else {
            List<String> skuList = new ArrayList<>();
            skuList.add(sku);
            querySkuDetailsAsync(skuList, (billingResult, skuDetailsList) -> {
                int responseCode = billingResult.getResponseCode();
                if (responseCode == BillingResponseCode.OK && skuDetailsList != null && !skuDetailsList.isEmpty()) {
                    buyProduct(skuDetailsList.get(0));
                } else {
                    Log.e(TAG, "Unable to get product details before buying: " + billingResult.getDebugMessage());
                    onPurchaseResult(responseCode, null);
                }
            });
        }
    }

    @Override
    public void consume(String purchaseToken) {
        Log.d(TAG, "consumePurchase() " + purchaseToken);
        ConsumeParams consumeParams = ConsumeParams.newBuilder()
                .setPurchaseToken(purchaseToken)
                .build();

        billingClient.consumeAsync(consumeParams, (billingResult, purchaseToken1) -> {
            Log.d(TAG, "finishTransaction() response code " + billingResult.getResponseCode() + " purchaseToken: " + purchaseToken1);
            // note: we only call the purchase listener if an error happens
            final int responseCode = billingResult.getResponseCode();
            if (responseCode != BillingResponseCode.OK) {
                Log.e(TAG, "Unable to consume purchase: " + billingResult.getDebugMessage());
                onPurchaseResult(responseCode, null);
            }
        });
    }

    @Override
    public void onDestroy() {
        // very important:
        Log.d(TAG, "Destroying helper.");
        if (billingClient.isReady()) {
            billingClient.endConnection();
        }
    }

    @Override
    public void onPurchasesUpdated(@NonNull BillingResult billingResult, @Nullable List<Purchase> purchases) {
        final int responseCode = billingResult.getResponseCode();
        if (responseCode == BillingResponseCode.OK && purchases != null) {
            for (Purchase purchase : purchases) {
                handlePurchase(purchase);
            }
        } else {
            onPurchaseResult(responseCode, null);
        }
    }

    /**
     * Handle a purchase. If the extension is configured to automatically
     * finish transactions the purchase will be immediately consumed. Otherwise
     * the product will be returned via the listener without being consumed.
     * NOTE: Billing 3.0 requires purchases to be acknowledged within 3 days of
     * purchase unless they are consumed.
     */
    private void handlePurchase(final Purchase purchase) {
        if (this.autoFinishTransactions) {
            consumePurchase(purchase.getPurchaseToken(), (ConsumeResponseListener) (billingResult, purchaseToken) -> {
                Log.d(TAG, "handlePurchase() response code " + billingResult.getResponseCode() + " purchaseToken: " + purchaseToken);
                onPurchaseResult(billingResult.getResponseCode(), purchase);
            });
        } else {
            onPurchaseResult(BillingResponseCode.OK, purchase);
        }
    }

    /**
     * Consume a purchase. This will acknowledge the purchase and make it
     * available to buy again.
     */
    private void consumePurchase(final String purchaseToken, final ConsumeResponseListener consumeListener) {
        Log.d(TAG, "consumePurchase() " + purchaseToken);
        ConsumeParams consumeParams = ConsumeParams.newBuilder()
                .setPurchaseToken(purchaseToken)
                .build();

        billingClient.consumeAsync(consumeParams, consumeListener);
    }

    private int convertPurchaseState(final int purchaseState) {
        int state = -1; // by default and unhandled: UNSPECIFIED_STATE
        switch (purchaseState) {
            case PurchaseState.PENDING:
                state = 1;
                break;
            case PurchaseState.PURCHASED:
                state = 0;
                break;
        }
        return state;
    }

    private void onPurchaseResult(final int responseCode, @Nullable final Purchase purchase) {
        EkActivity.runGLThread(() -> {
            try {
                String productId = pendingPurchase;
                String token = "";
                String payload = "";
                int state = -1;
                String dataSignature = "";

                if (purchase != null) {
                    token = purchase.getPurchaseToken();
                    payload = purchase.getDeveloperPayload();
                    state = convertPurchaseState(purchase.getPurchaseState());
                    dataSignature = purchase.getSignature();

                    // handle already owned error
                    if (responseCode == BillingResponseCode.ITEM_ALREADY_OWNED) {
                        state = 0;
                    }

                    // TODO: change protocol
                    ArrayList<String> skus = purchase.getSkus();
                    if (skus.size() > 0) {
                        productId = skus.get(0);
                    }
                    //for(String sku : purchase.getSkus()) {
                    //}
                }
                BillingBridge.nativePurchase(productId, token, state, payload, dataSignature, responseCode);
            } catch (Exception e) {
                e.printStackTrace();
            }
            pendingPurchase = null;
        });
    }

    /**
     * Get details for a list of products. The products can be a mix of
     * in-app products and subscriptions.
     */
    private void querySkuDetailsAsync(final List<String> skuList, final SkuDetailsResponseListener listener) {
        SkuDetailsResponseListener detailsListener = new SkuDetailsResponseListener() {
            private final List<SkuDetails> allSkuDetails = new ArrayList<>();
            private int queries = 2;

            @Override
            public void onSkuDetailsResponse(@NonNull BillingResult billingResult, List<SkuDetails> skuDetails) {
                if (skuDetails != null) {
                    // cache skus (cache will be used to speed up buying)
                    for (SkuDetails sd : skuDetails) {
                        products.put(sd.getSku(), sd);
                    }
                    // add to list of all sku details
                    allSkuDetails.addAll(skuDetails);
                }
                // we're finished when we have queried for both in-app and subs
                queries--;
                if (queries == 0) {
                    listener.onSkuDetailsResponse(billingResult, allSkuDetails);
                }
            }
        };
        billingClient.querySkuDetailsAsync(SkuDetailsParams.newBuilder().setSkusList(skuList).setType(SkuType.INAPP).build(), detailsListener);
        billingClient.querySkuDetailsAsync(SkuDetailsParams.newBuilder().setSkusList(skuList).setType(SkuType.SUBS).build(), detailsListener);
    }

    /**
     * Buy a product. This method stores the listener and uses it in the
     * onPurchasesUpdated() callback.
     */
    private void buyProduct(SkuDetails sku) {
        BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                .setSkuDetails(sku)
                .build();

        BillingResult billingResult = billingClient.launchBillingFlow(this.activity, billingFlowParams);
        int responseCode = billingResult.getResponseCode();
        if (responseCode != BillingResponseCode.OK) {
            Log.e(TAG, "Purchase failed: " + billingResult.getDebugMessage());
            onPurchaseResult(responseCode, null);
        }
    }

    private void queryPurchases(@NonNull final String type) {
        billingClient.queryPurchasesAsync(type, (billingResult, purchases) -> {
            for (Purchase purchase : purchases) {
                handlePurchase(purchase);
            }
            if (billingResult.getResponseCode() != BillingResponseCode.OK) {
                Log.e(TAG, "Unable to query pending purchases: " + billingResult.getDebugMessage());
            }
        });
    }
}
