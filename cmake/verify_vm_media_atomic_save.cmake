if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdd.c" fdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/hdd.c" hdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/media_save.c" save_source)

foreach(source_name IN ITEMS fdd_source hdd_source)
    if(NOT "${${source_name}}" MATCHES "vm/machine/media_save.h" OR
        NOT "${${source_name}}" MATCHES "vm_machine_media_save_atomically")
        message(FATAL_ERROR "${source_name} bypasses VM atomic media persistence")
    endif()
    if("${${source_name}}" MATCHES "STD_FOPEN\\(file_name, \\\"wb\\\"\\)")
        message(FATAL_ERROR "${source_name} still truncates a media target directly")
    endif()
endforeach()

foreach(required IN ITEMS "STD_FOPEN_EXCLUSIVE_WRITE" "STD_FWRITE" "STD_FCLOSE" "STD_RENAME_REPLACE"
    "STD_REMOVE")
    if(NOT "${save_source}" MATCHES "${required}")
        message(FATAL_ERROR "VM atomic media save omits ${required}")
    endif()
endforeach()

message("M5:T283:S6:VM-MEDIA-ATOMIC-SAVE:OK")
