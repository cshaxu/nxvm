if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE app_test_sources
    "${PROJECT_SOURCE_DIR}/test/app-*/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-*/*.h")
foreach(app_test_source IN LISTS app_test_sources)
    file(RELATIVE_PATH app_test_relative_path
        "${PROJECT_SOURCE_DIR}/test" "${app_test_source}")
    if(NOT app_test_relative_path MATCHES "^app-([^/]+)/")
        continue()
    endif()
    set(app_test_owner "${CMAKE_MATCH_1}")
    file(READ "${app_test_source}" app_test_text)
    string(REGEX MATCHALL
        "#[ \t]*include[ \t]*[<\"]test/app-([^/]+)/"
        app_test_includes "${app_test_text}")
    foreach(app_test_include IN LISTS app_test_includes)
        string(REGEX REPLACE ".*test/app-([^/]+)/.*" "\\1"
            app_test_include_owner "${app_test_include}")
        if(NOT app_test_include_owner STREQUAL app_test_owner)
            message(FATAL_ERROR
                "App test support crosses owners: ${app_test_relative_path} -> "
                "app-${app_test_include_owner}")
        endif()
    endforeach()
endforeach()
message(STATUS "APP-TEST-SUPPORT-BOUNDARY:OK")
