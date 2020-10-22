
target_sources(${PROJECT_NAME} PUBLIC

        common/components.hpp
        common/identity_generator_test_1.cpp
        common/identity_generator_test_2.cpp
        common/identity_generator_shared.hpp

        entity_pool_test.cpp
        entity_map_test.cpp
        bit_count_test.cpp
        sparse_vector_test.cpp
        entity_value_test.cpp
        world_test.cpp
        components_test.cpp
        view_test.cpp)