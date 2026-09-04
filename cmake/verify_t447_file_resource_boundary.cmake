if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/src/core/platform/file.c" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/src/core/platform/file.h")
    message(FATAL_ERROR "T447 S4 Core file owner is missing.")
endif()
foreach(project_t447_retired IN ITEMS
    "src/core/platform/backing_resource_interface.c"
    "src/core/platform/backing_resource_interface.h")
    if(EXISTS "${PROJECT_SOURCE_DIR}/${project_t447_retired}")
        message(FATAL_ERROR "T447 S4 retained obsolete backing-resource route: ${project_t447_retired}")
    endif()
endforeach()

foreach(project_t447_consumer IN ITEMS
    "src/vm/machine/fdd.c"
    "src/vm/machine/hdd.c"
    "src/vm/machine/media_save.c"
    "src/vm/machine/debug.c"
    "src/vm/product/session_catalog.c")
    file(READ "${PROJECT_SOURCE_DIR}/${project_t447_consumer}" project_t447_source)
    if(NOT project_t447_source MATCHES "core/platform/file.h")
        message(FATAL_ERROR "T447 S4 consumer bypasses Core file route: ${project_t447_consumer}")
    endif()
    foreach(project_t447_facade IN ITEMS "STD_FOPEN" "STD_FCLOSE" "STD_FREAD"
        "STD_FWRITE" "STD_FGETS" "STD_FPRINTF" "STD_RENAME_REPLACE" "STD_REMOVE")
        if(project_t447_source MATCHES "${project_t447_facade}")
            message(FATAL_ERROR "T447 S4 consumer retains direct file facade ${project_t447_facade}: ${project_t447_consumer}")
        endif()
    endforeach()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" project_t447_cmake)
if(project_t447_cmake MATCHES "vm-debugger-recording-lifecycle-smoke[\\s\\S]*src/vm/machine/debug.c" OR
        NOT project_t447_cmake MATCHES "target_link_libraries\\(vm-debugger-recording-lifecycle-smoke PRIVATE vm-machine\\)")
    message(FATAL_ERROR "T447 S4 recorder smoke does not use production vm-machine linkage.")
endif()
message("M5:T447:S4:FILE-RESOURCE-BOUNDARY:OK")
