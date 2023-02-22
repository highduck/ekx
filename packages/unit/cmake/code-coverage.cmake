
function(target_code_coverage TARGET_NAME)
    set(options AUTO ALL EXTERNAL PUBLIC INTERFACE)
    set(single_value_keywords COVERAGE_TARGET_NAME)
    set(multi_value_keywords EXCLUDE OBJECTS ARGS)
    cmake_parse_arguments(target_code_coverage "${options}" "${single_value_keywords}" "${multi_value_keywords}" ${ARGN})
    if (target_code_coverage_PUBLIC)
        set(TARGET_VISIBILITY PUBLIC)
    elseif (target_code_coverage_INTERFACE)
        set(TARGET_VISIBILITY INTERFACE)
    else ()
        set(TARGET_VISIBILITY PRIVATE)
    endif ()

    if (NOT target_code_coverage_COVERAGE_TARGET_NAME)
        set(target_code_coverage_COVERAGE_TARGET_NAME ${TARGET_NAME})
    endif ()

    if (CODE_COVERAGE)
        target_compile_options(${TARGET_NAME} ${TARGET_VISIBILITY} -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${TARGET_NAME} ${TARGET_VISIBILITY} -fprofile-instr-generate -fcoverage-mapping)
    endif ()
endfunction()

function(test_code_coverage TARGET_NAME)
    set(options)
    set(oneValueArg NAME)
    set(multiValueArgs ARGS)
    cmake_parse_arguments(test_code_coverage "${options}" "${oneValueArg}"
            "${multiValueArgs}" ${ARGN})
    if (CODE_COVERAGE)
        if (NOT test_code_coverage_ARGS)
            set(test_code_coverage_ARGS "")
        endif ()

        if (NOT test_code_coverage_NAME)
            set(test_code_coverage_NAME ${TARGET_NAME})
        endif ()

        list(APPEND COVERAGE_COLLECT_TARGETS coverage-collect-${test_code_coverage_NAME})
        set(COVERAGE_COLLECT_TARGETS ${COVERAGE_COLLECT_TARGETS} CACHE INTERNAL "COVERAGE_COLLECT_TARGETS")

        list(APPEND COVERAGE_RAW_PROFILES ${CMAKE_BINARY_DIR}/${test_code_coverage_NAME}.profraw)
        set(COVERAGE_RAW_PROFILES ${COVERAGE_RAW_PROFILES} CACHE INTERNAL "COVERAGE_RAW_PROFILES")

        list(APPEND COVERAGE_OBJECT_FILES -object $<TARGET_FILE:${TARGET_NAME}>)
        set(COVERAGE_OBJECT_FILES ${COVERAGE_OBJECT_FILES} CACHE INTERNAL "COVERAGE_OBJECT_FILES")

        add_custom_target(coverage-collect-${test_code_coverage_NAME}
                COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE="${CMAKE_BINARY_DIR}/${test_code_coverage_NAME}.profraw" $<TARGET_FILE:${TARGET_NAME}> ${test_code_coverage_ARGS} || (exit 0)
                )
    endif ()
endfunction()

function(coverage_all)
    if (CODE_COVERAGE)
        if (${CMAKE_SYSTEM_NAME} MATCHES "iOS|Darwin")
            set(LLVM_PROFDATA xcrun llvm-profdata)
            set(LLVM_COV xcrun llvm-cov)
        else ()
            set(LLVM_PROFDATA llvm-profdata)
            set(LLVM_COV llvm-cov)
        endif ()
        add_custom_target(coverage
                COMMAND ${LLVM_PROFDATA} merge -sparse ${COVERAGE_RAW_PROFILES} -o ${CMAKE_BINARY_DIR}/coverage.profdata
                COMMAND ${LLVM_COV} show ${COVERAGE_OBJECT_FILES} -instr-profile=${CMAKE_BINARY_DIR}/coverage.profdata > ${CMAKE_BINARY_DIR}/coverage.txt
                COMMAND ${LLVM_COV} report ${COVERAGE_OBJECT_FILES} -instr-profile=${CMAKE_BINARY_DIR}/coverage.profdata -use-color=1
                DEPENDS ${COVERAGE_COLLECT_TARGETS}
                )
    endif ()
endfunction()