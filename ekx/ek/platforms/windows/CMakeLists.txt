message("Platform: Windows")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE app_windows.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE -mwin32)

target_link_libraries(${PROJECT_NAME}
        PRIVATE kernel32
        PRIVATE user32
        PRIVATE shell32
        PRIVATE d3d11
        PRIVATE dxgi
        PRIVATE dxguid
        )