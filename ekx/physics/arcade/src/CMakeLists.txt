add_library(${PROJECT_NAME} STATIC ArcadePhysicsModule.cpp
        collisions.hpp
        QuadTree.hpp
        RegularGrid.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC .)