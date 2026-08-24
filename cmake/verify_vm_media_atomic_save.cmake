if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdd.c" fdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/hdd.c" hdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/media_save.c" save_source)

foreach(source_name IN ITEMS hdd_source)
    if(NOT "${${source_name}}" MATCHES "vm/machine/media_save.h" OR
        NOT "${${source_name}}" MATCHES "vm_machine_media_save_atomically")
        message(FATAL_ERROR "${source_name} bypasses VM atomic media persistence")
    endif()
    if("${${source_name}}" MATCHES "STD_FOPEN\\(file_name, \\\"wb\\\"\\)")
        message(FATAL_ERROR "${source_name} still truncates a media target directly")
    endif()
endforeach()

if(NOT "${fdd_source}" MATCHES "vm/machine/media_save.h" OR
    NOT "${fdd_source}" MATCHES "vm_machine_media_save_pair_atomically")
    message(FATAL_ERROR "fdd_source bypasses paired VM atomic media persistence")
endif()
string(FIND "${fdd_source}" "STD_FOPEN(file_name, \"wb\")" fdd_direct_position)
if(NOT fdd_direct_position EQUAL -1)
    message(FATAL_ERROR "fdd_source still truncates a media target directly")
endif()

foreach(required IN ITEMS "core/platform/file.h" "core_platform_file_write_exclusive"
    "core_platform_file_replace" "core_platform_file_remove")
    if(NOT "${save_source}" MATCHES "${required}")
        message(FATAL_ERROR "VM atomic media save omits ${required}")
    endif()
endforeach()
foreach(forbidden IN ITEMS "STD_FOPEN_EXCLUSIVE_WRITE" "STD_FWRITE" "STD_FCLOSE"
    "STD_RENAME_REPLACE" "STD_REMOVE")
    if("${save_source}" MATCHES "${forbidden}")
        message(FATAL_ERROR "VM atomic media save bypasses Core file ownership: ${forbidden}")
    endif()
endforeach()

message("M5:T283:S6:VM-MEDIA-ATOMIC-SAVE:OK")
