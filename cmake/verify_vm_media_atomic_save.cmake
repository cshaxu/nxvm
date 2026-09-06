if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdd.c" fdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/hdd.c" hdd_source)

foreach(source_name IN ITEMS fdd_source hdd_source)
    if(NOT "${${source_name}}" MATCHES "lib/storage/file.h" OR
        NOT "${${source_name}}" MATCHES "lib_storage_commit")
        message(FATAL_ERROR "${source_name} bypasses lib storage atomic persistence")
    endif()
    foreach(forbidden IN ITEMS "STD_FOPEN_EXCLUSIVE_WRITE" "STD_FWRITE" "STD_FCLOSE"
        "STD_RENAME_REPLACE" "STD_REMOVE" "core_platform_file")
        if("${${source_name}}" MATCHES "${forbidden}")
            message(FATAL_ERROR "${source_name} retains obsolete persistence facade ${forbidden}")
        endif()
    endforeach()
endforeach()

message("M5:T283:S6:VM-MEDIA-ATOMIC-SAVE:OK")
