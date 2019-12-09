mergeInto(LibraryManager.library, {
    web_set_mouse_cursor: function (cursor) {
        var PARENT = 0;
        var ARROW = 1;
        var BUTTON = 2;
        var HELP = 3;
        var map = [
            "auto", // 0
            "default", // 1
            "pointer", // 2
            "help" // 3
        ];
        cursor = cursor | 0;
        if (cursor >= 0 && cursor < map.length) {
            gameview = document.getElementById("gameview");
            if (gameview) {
                gameview.style.cursor = map[cursor];
            }
        }
    },
    web_update_gameview_size: function (width, height, dpr, offsetX, offsetY) {
        var gameview = document.getElementById("gameview");
        if (gameview) {
            var drawableWidth = (width * dpr) | 0;
            var drawableHeight = (height * dpr) | 0;
            if (gameview.width !== drawableWidth ||
                gameview.height !== drawableHeight) {
                gameview.width = (width * dpr) | 0;
                gameview.height = (height * dpr) | 0;
            }
            gameview.style.width = width + "px";
            gameview.style.height = height + "px";
            gameview.style.transform = "translateX(" + offsetX + "px) translateY(" + offsetY + "px)";
        }
    },

    // AUDIO - MINI
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
        if (sound) {
            var id = sound.play();
            sound.volume(vol, id);
        }
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
});