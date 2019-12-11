add_library(cocos-audio-ios STATIC
        CDAudioManager.h
        CDAudioManager.mm
        CDConfig.h
        CDOpenALSupport.h
        CDOpenALSupport.mm
        CocosDenshion.h
        CocosDenshion.mm
        SimpleAudioEngine_objc.h
        SimpleAudioEngine_objc.mm
        )

target_link_libraries(cocos-audio-ios
        "-framework AudioToolbox"
        "-framework Foundation"
        "-framework OpenAL"
        )