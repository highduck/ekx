package ek;

import android.content.Context;
import android.content.SharedPreferences;

import androidx.annotation.Keep;

@Keep
public class LocalStorage {

    final private static String TAG = "LocalStorage";
    final private static String ID = "user";

    private static SharedPreferences _prefs;

    private static SharedPreferences getSharedPreferences() {
        if (_prefs == null) {
            _prefs = EkActivity.getActivity().getSharedPreferences(ID, Context.MODE_PRIVATE);
        }
        return _prefs;
    }

    @Keep
    public static void set_int(String key, int value) {
        SharedPreferences.Editor editor = getSharedPreferences().edit();
        editor.putInt(key, value);
        editor.apply();
    }

    @Keep
    public static int get_int(String key, int def) {
        try {
            return getSharedPreferences().getInt(key, def);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        return def;
    }

    @Keep
    public static void set_string(String key, String value) {
        SharedPreferences.Editor editor = getSharedPreferences().edit();
        editor.putString(key, value);
        editor.apply();
    }

    @Keep
    public static String get_string(String key) {
        try {
            return getSharedPreferences().getString(key, "");
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        return "";
    }
}
