if(NOT DEFINED PROJECT_APP_TEST_OWNER)
    message(FATAL_ERROR "App test support boundary requires an App owner.")
endif()

file(GLOB_RECURSE project_app_test_sources
    "${CMAKE_SOURCE_DIR}/test/${PROJECT_APP_TEST_OWNER}/*.c"
    "${CMAKE_SOURCE_DIR}/test/${PROJECT_APP_TEST_OWNER}/*.h"
    "${CMAKE_SOURCE_DIR}/test/${PROJECT_APP_TEST_OWNER}/*.cmake")
foreach(project_app_test_source IN LISTS project_app_test_sources)
    file(STRINGS "${project_app_test_source}" project_app_test_includes
        REGEX "^[ \t]*#[ \t]*include.*app-(nxvm|my5160|my5170|mydeskpro386)/support")
    foreach(project_app_test_include IN LISTS project_app_test_includes)
        if(project_app_test_include MATCHES "app-([a-z0-9]+)/support" AND
                NOT "app-${CMAKE_MATCH_1}" STREQUAL "${PROJECT_APP_TEST_OWNER}")
            message(FATAL_ERROR
                "${PROJECT_APP_TEST_OWNER} test depends on peer App support: "
                "${project_app_test_source}: ${project_app_test_include}")
        endif()
    endforeach()
endforeach()
message(STATUS "${PROJECT_APP_TEST_OWNER} test support boundary: OK")
