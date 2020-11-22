package ek.billing;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import androidx.annotation.Keep;

import com.android.vending.billing.IInAppBillingService;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

import ek.EkActivity;

@Keep
public class GooglePlayBilling extends BillingPlugin {

    final private static String TAG = "GooglePlayBilling";

    IInAppBillingService service;

    String pendingPurchase;

    ServiceConnection connection = new ServiceConnection() {
        @Override
        public void onServiceDisconnected(ComponentName name) {
            service = null;
            Log.d(TAG, "Service disconnected");
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            service = IInAppBillingService.Stub.asInterface(binder);
            Log.d(TAG, "Service connected");
        }
    };

    public GooglePlayBilling(Activity activity, String developerKey) {
        super(activity, developerKey);
        Intent serviceIntent = new Intent("com.android.vending.billing.InAppBillingService.BIND");
        serviceIntent.setPackage("com.android.vending");
        activity.bindService(serviceIntent, connection, Context.BIND_AUTO_CREATE);
    }

    @Override
    public String getName() {
        return "GooglePlay";
    }

    @Override
    public void getPurchases() {
        new Thread(() -> {
            if (service == null) {
                return;
            }

            Log.d(TAG, "getPurchases");
            try {
                Bundle purchases = service.getPurchases(3, activity.getPackageName(), "inapp", null);
                int response = purchases.getInt("RESPONSE_CODE");
                if (response == 0) {
                    EkActivity.runGLThread(() -> {
                        final ArrayList<String> details = purchases.getStringArrayList("INAPP_PURCHASE_DATA_LIST");
                        final ArrayList<String> signatures = purchases.getStringArrayList("INAPP_DATA_SIGNATURE_LIST");

                        int num = details.size();
                        if (num > 0) {
                            for (int i = 0; i < num; ++i) {
                                try {
                                    final JSONObject o = new JSONObject(details.get(i));
                                    final String productId = o.optString("productId");
                                    final String token = o.optString("token", o.optString("purchaseToken"));
                                    final int state = o.optInt("purchaseState");
                                    final String payload = o.optString("developerPayload");

                                    BillingBridge.nativePurchase(productId, token, state, payload, signatures.get(i), 0);
                                } catch (JSONException e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                    });
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }

        }).start();
    }

    @Override
    public void getDetails(String[] skus) {
        final ArrayList<String> skuList = new ArrayList<>();
        for (String sku : skus) {
            skuList.add(sku);
        }

        new Thread(() -> {
            try {
                if (service == null) {
                    Log.d(TAG, "getDetails failed service is null");
                    return;
                }

                Log.d(TAG, "getDetails");

                ArrayList<String> details = new ArrayList<>();

                for (int i = 0; i < skuList.size(); i += 20) {
                    ArrayList<String> items = new ArrayList<>(skuList.subList(i, Math.min(i + 20, skuList.size())));

                    Bundle querySkus = new Bundle();
                    querySkus.putStringArrayList("ITEM_ID_LIST", items);

                    Bundle skuDetails = service.getSkuDetails(3, activity.getPackageName(), "inapp", querySkus);
                    int response = skuDetails.getInt("RESPONSE_CODE");
                    if (response == 0) {
                        details.addAll(skuDetails.getStringArrayList("DETAILS_LIST"));
                    }
                }

                EkActivity.runGLThread(() -> {
                    for (String detailsData : details) {
                        try {
                            final JSONObject object = new JSONObject(detailsData);
                            final String sku = object.getString("productId");
                            final String price = object.getString("price");
                            final String currencyCode = object.getString("price_currency_code");
                            BillingBridge.nativeDetails(sku, price, currencyCode);
                        } catch (JSONException e) {
                            e.printStackTrace();
                        }
                    }
                });

            } catch (Exception e) {
                e.printStackTrace();
            }
        }).start();
    }

    @Override
    public void purchase(String sku, String payload) {
        pendingPurchase = sku;
        activity.runOnUiThread(() -> {
            if (service == null) {
                Log.d(TAG, "purchase failed service is null");
                return;
            }

            try {
                Log.d(TAG, "purchase");
                //item = "android.test.purchased";
                Bundle buyIntentBundle = service.getBuyIntent(3, activity.getPackageName(), sku, "inapp", payload);
                PendingIntent pendingIntent = buyIntentBundle.getParcelable("BUY_INTENT");

                activity.startIntentSenderForResult(pendingIntent.getIntentSender(),
                        RC_REQUEST, new Intent(), 0, 0, 0);
            } catch (Exception e) {
                e.printStackTrace();
            }
        });
    }

    @Override
    public void consume(String token) {
        new Thread(() -> {
            if (service == null) {
                Log.d(TAG, "consume failed service is null");
                return;
            }

            try {
                Log.d(TAG, "consume");
                int response = service.consumePurchase(3, activity.getPackageName(), token);
                // TODO: ok
                //if (response == 0) {
                //}
            } catch (Exception e) {
                e.printStackTrace();
            }
        }).start();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == RC_REQUEST) {
            try {
                int responseCode = data.getIntExtra("RESPONSE_CODE", 0);
                String productId = pendingPurchase;
                String token = "";
                int state = -1;
                String payload = "";

                String purchaseData = data.getStringExtra("INAPP_PURCHASE_DATA");
                if (purchaseData != null) {
                    final JSONObject o = new JSONObject(purchaseData);
                    productId = o.optString("productId");
                    token = o.optString("token", o.optString("purchaseToken"));
                    state = o.optInt("purchaseState");
                    payload = o.optString("developerPayload");
                }

                String dataSignature = data.getStringExtra("INAPP_DATA_SIGNATURE");
                if (dataSignature == null) {
                    dataSignature = "";
                }

                // handle already owned error
                if (responseCode == 7 /* BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED */) {
                    state = 0;
                }
                BillingBridge.nativePurchase(productId, token, state, payload, dataSignature, responseCode);
            } catch (Exception e) {
                e.printStackTrace();
            }
            pendingPurchase = null;
        }
    }

    @Override
    public void onDestroy() {
        // very important:
        Log.d(TAG, "Destroying helper.");
        if (service != null) {
            activity.unbindService(connection);
            // service = null ?
        }
    }

    boolean verify(String data, String signature) {
        return true;
    }
}
