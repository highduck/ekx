package ekapp;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.SoundPool;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.support.annotation.Keep;
import android.util.Log;

import java.io.IOException;
import java.util.HashMap;

public class EKAudio {

    final private static String TAG = "EKAudio";

    private final static int MAX_SIMULTANEOUS_SOUNDS = 8;

    private static AssetManager _assets;
    private static AudioManager _audio;
    private static Vibrator _vibrator;

    private static SoundPool _soundPool;
    public static EKMusic _gameMusic;

    private final static HashMap<String, Integer> _soundsMap = new HashMap<>();

    public static void register(Context context) {
        Log.d(TAG, "register");
        _assets = context.getAssets();
        _audio = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            AudioAttributes audioAttrib = new AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_GAME)
                    .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                    .build();
            _soundPool = new SoundPool.Builder().setAudioAttributes(audioAttrib).setMaxStreams(MAX_SIMULTANEOUS_SOUNDS).build();
        } else {
            _soundPool = new SoundPool(MAX_SIMULTANEOUS_SOUNDS, AudioManager.STREAM_MUSIC, 0);
            // srcQuality: the sample-rate converter quality. Currently has no effect. Use 0 for the default.
        }
        if (context instanceof Activity) {
            ((Activity) context).setVolumeControlStream(AudioManager.STREAM_MUSIC);
        }
        _vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
    }

    @Keep
    public static void playSound(String name, float vol, float pan) {
        if (_soundsMap.containsKey(name)) {
            final int id = _soundsMap.get(name);
            _soundPool.play(id, 0.5f * vol, 0.5f * vol, 0, 0, 1.0f);
        }
    }

    @Keep
    public static void playMusic(String name, float gain) {
        if (_gameMusic != null) {
            _gameMusic.playing = true;
            _gameMusic.volume = 0.5f * gain;
        }
    }

    @Keep
    public static void createSound(String name) {
        try {
            AssetFileDescriptor fd = _assets.openFd(name);
            final int id = _soundPool.load(fd, 1);
            _soundsMap.put(name, id);
        } catch (java.io.IOException e) {
            Log.w("hl", "createSound", e);
        }
    }

    @Keep
    public static void createMusic(String name) {
        if (_gameMusic == null) {
            try {
                AssetFileDescriptor afd = _assets.openFd(name);
                if (afd != null) {
                    _gameMusic = new EKMusic(afd);
                }
            } catch (IOException e) {

            }
        }
    }

    public static void onPause() {
        if (_gameMusic != null && _gameMusic.audioTrack != null && _gameMusic.audioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
            _gameMusic.audioTrack.pause();
        }
    }

    public static void onResume() {
        if (_gameMusic != null && _gameMusic.audioTrack != null && _gameMusic.audioTrack.getPlayState() == AudioTrack.PLAYSTATE_PAUSED) {
            _gameMusic.audioTrack.play();
        }
    }

    @Keep
    public static void vibrate(long durationMillis) {
        if (_vibrator != null) {
            if (Build.VERSION.SDK_INT >= 26) {
                _vibrator.vibrate(VibrationEffect.createOneShot(durationMillis, VibrationEffect.DEFAULT_AMPLITUDE));
            } else {
                _vibrator.vibrate(durationMillis);
            }
        }
    }

//    @Keep
//    public static native void audio_on_music_fft(byte[] bytes);

    public static void destroy() {
        _soundsMap.clear();

        if (_soundPool != null) {
            _soundPool.release();
            _soundPool = null;
        }
        if (_gameMusic != null) {
            _gameMusic.release();
            _gameMusic = null;
        }

        _vibrator = null;
    }
}
