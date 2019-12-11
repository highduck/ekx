add_library(cocos-audio-mac STATIC
        CDAudioManager.h
        CDAudioManager.mm
        CDConfig.h
        CDOpenALSupport.h
        CDOpenALSupport.mm
        CDXMacOSXSupport.h
        CDXMacOSXSupport.mm
        CocosDenshion.h
        CocosDenshion.mm
        SimpleAudioEngine_objc.h
        SimpleAudioEngine_objc.mm
        )

target_link_libraries(cocos-audio-mac
        "-framework AudioToolbox"
        "-framework OpenAL"
        )