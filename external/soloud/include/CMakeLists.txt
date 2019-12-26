target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PUBLIC
        soloud.h
        soloud_audiosource.h
        soloud_bassboostfilter.h
        soloud_biquadresonantfilter.h
        soloud_bus.h
        soloud_c.h
        soloud_dcremovalfilter.h
        soloud_echofilter.h
        soloud_error.h
        soloud_fader.h
        soloud_fft.h
        soloud_fftfilter.h
        soloud_file.h
        soloud_file_hack_off.h
        soloud_file_hack_on.h
        soloud_filter.h
        soloud_flangerfilter.h
        soloud_internal.h
        soloud_lofifilter.h
        soloud_monotone.h
        soloud_openmpt.h
        soloud_queue.h
        soloud_robotizefilter.h
        soloud_sfxr.h
        soloud_speech.h
        soloud_tedsid.h
        soloud_thread.h
        soloud_vic.h
        soloud_vizsn.h
        soloud_wav.h
        soloud_waveshaperfilter.h
        soloud_wavstream.h
        )

