
target_sources(${PROJECT_NAME} PUBLIC
        core/soloud.cpp
        core/soloud_audiosource.cpp
        core/soloud_bus.cpp
        core/soloud_core_3d.cpp
        core/soloud_core_basicops.cpp
        core/soloud_core_faderops.cpp
        core/soloud_core_filterops.cpp
        core/soloud_core_getters.cpp
        core/soloud_core_setters.cpp
        core/soloud_core_voicegroup.cpp
        core/soloud_core_voiceops.cpp
        core/soloud_fader.cpp
        core/soloud_fft.cpp
        core/soloud_fft_lut.cpp
        core/soloud_file.cpp
        core/soloud_filter.cpp
        core/soloud_queue.cpp
        core/soloud_thread.cpp

        backend/alsa/soloud_alsa.cpp

        backend/coreaudio/soloud_coreaudio.cpp
        backend/null/soloud_null.cpp
        backend/openal/soloud_openal.cpp
        backend/openal/soloud_openal_dll.c
        backend/opensles/soloud_opensles.cpp
        backend/oss/soloud_oss.cpp
        backend/portaudio/soloud_portaudio.cpp
        #        backend/portaudio/soloud_portaudio_dll.c
#                backend/sdl/soloud_sdl1.cpp
        #        backend/sdl/soloud_sdl1_dll.c
        #        backend/sdl/soloud_sdl2.cpp
        #        backend/sdl/soloud_sdl2_dll.c
                backend/sdl2_static/soloud_sdl2_static.cpp
                backend/sdl_static/soloud_sdl_static.cpp
        #        backend/vita_homebrew/soloud_vita_homebrew.cpp
        backend/wasapi/soloud_wasapi.cpp
        backend/winmm/soloud_winmm.cpp
        backend/xaudio2/soloud_xaudio2.cpp


        # speech
        audiosource/speech/darray.cpp
        audiosource/speech/darray.h
        audiosource/speech/Elements.def
        audiosource/speech/klatt.cpp
        audiosource/speech/klatt.h
        audiosource/speech/legal_readme.txt
        audiosource/speech/resonator.cpp
        audiosource/speech/resonator.h
        audiosource/speech/soloud_speech.cpp
        audiosource/speech/tts.cpp
        audiosource/speech/tts.h

        # wav
        audiosource/wav/dr_flac.h
        audiosource/wav/dr_impl.cpp
        audiosource/wav/dr_mp3.h
        audiosource/wav/dr_wav.h
        audiosource/wav/soloud_wav.cpp
        audiosource/wav/soloud_wavstream.cpp
        audiosource/wav/stb_vorbis.h
        audiosource/wav/stb_vorbis.c
        )
