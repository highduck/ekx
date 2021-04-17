package ek.admob;

import android.app.Activity;
import android.util.Log;

import androidx.annotation.Keep;

import com.google.android.ump.ConsentDebugSettings;
import com.google.android.ump.ConsentForm;
import com.google.android.ump.ConsentInformation;
import com.google.android.ump.ConsentRequestParameters;
import com.google.android.ump.UserMessagingPlatform;

import ek.EkActivity;

import static com.google.android.gms.ads.AdRequest.DEVICE_ID_EMULATOR;

@Keep
public class UMP {

    public static ConsentInformation consentInformation;
    public static ConsentForm consentForm;

    public static void start() {
        final Activity activity = EkActivity.getInstance();

        ConsentDebugSettings debugSettings = new ConsentDebugSettings.Builder(activity)
//                .setDebugGeography(ConsentDebugSettings.DebugGeography.DEBUG_GEOGRAPHY_DISABLED)
                .setDebugGeography(ConsentDebugSettings.DebugGeography.DEBUG_GEOGRAPHY_EEA)
                .addTestDeviceHashedId(DEVICE_ID_EMULATOR)
                .addTestDeviceHashedId("14C5869A0FDC6FC70076993D3E303657")
                .build();

        // Set tag for underage of consent. false means users are not underage.
        ConsentRequestParameters params = new ConsentRequestParameters.Builder()
                //.setTagForUnderAgeOfConsent(false)
                .setConsentDebugSettings(debugSettings)
                .build();

        consentInformation = UserMessagingPlatform.getConsentInformation(activity);


        // DEBUG
//        consentInformation.reset();

        consentInformation.requestConsentInfoUpdate(activity, params,
                () -> {
                    int status = consentInformation.getConsentStatus();
                    // The consent information state was updated.
                    // You are now ready to check if a form is available.
                    if (consentInformation.isConsentFormAvailable()) {
                        loadForm();
                    }
                },
                formError -> {
                    // Handle the error.
                });
    }

    public static void loadForm() {
        final Activity activity = EkActivity.getInstance();
        UserMessagingPlatform.loadConsentForm(
                activity,
                consentForm -> {
                    UMP.consentForm = consentForm;
                    if (consentInformation.getConsentStatus() == ConsentInformation.ConsentStatus.REQUIRED) {
                        consentForm.show(
                                activity,
                                formError -> {
                                    // Handle dismissal by reloading form.
                                    loadForm();
                                });
                    }
                },
                formError -> {
                    // Handle the error
                }
        );
    }
}
