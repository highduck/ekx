var AudioMiniLib = {

    $AudioMini: {
        table: {},
        music: null,
        music_id: null,
        howl_create: function (name) {
            var howl = window["Howl"];
            return new howl({src: [name], html5: true});
        }
    },

    audio_mini_vibrate: function (millis) {
        if (navigator.vibrate) {
            navigator.vibrate(millis);
        }
    },

    audio_mini_create_sound__deps: ['$AudioMini'],
    audio_mini_create_sound: function (name) {
        name = UTF8ToString(name);
        AudioMini.table[name] = AudioMini.howl_create(name);
    },

    audio_mini_create_music__deps: ['$AudioMini'],
    audio_mini_create_music: function (name) {
        name = UTF8ToString(name);
        var sound = AudioMini.howl_create(name);
        sound.loop(true);
        AudioMini.table[name] = sound;
    },

    audio_mini_play_sound__deps: ['$AudioMini'],
    audio_mini_play_sound: function (name, vol, pan) {
        name = UTF8ToString(name);
        var sound = AudioMini.table[name];
        var id = sound.play();
        sound.volume(vol, id);
    },

    audio_mini_play_music__deps: ['$AudioMini'],
    audio_mini_play_music: function (name, vol) {
        name = UTF8ToString(name);
        var next = name ? AudioMini.table[name] : null;
        if (AudioMini.music !== next) {
            if (AudioMini.music && AudioMini.music_id) {
                AudioMini.music.stop(AudioMini.music_id);
            }
            AudioMini.music = next;
            AudioMini.music_id = null;
            if (next) {
                AudioMini.music_id = next.play();
            }
        }
        if (AudioMini.music && AudioMini.music_id) {
            AudioMini.music.volume(vol, AudioMini.music_id);
        }
    }
};

autoAddDeps(AudioMiniLib, '$AudioMini');
mergeInto(LibraryManager.library, AudioMiniLib);