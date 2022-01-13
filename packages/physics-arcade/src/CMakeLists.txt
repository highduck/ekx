add_library(${PROJECT_NAME} STATIC
        ArcadePhysicsModule.cpp
        QuadTree.hpp
        RegularGrid.hpp)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC .)