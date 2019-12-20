target_include_directories(${PROJECT_NAME} PRIVATE .)
target_sources(${PROJECT_NAME} PRIVATE
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

target_link_libraries(${PROJECT_NAME}
        "-framework AudioToolbox"
        "-framework Foundation"
        "-framework OpenAL"
        )