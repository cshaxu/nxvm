if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT EXISTS "${PROJECT_SOURCE_DIR}/test/integration")
    message(FATAL_ERROR "T533 integration source root is required.")
endif()

file(GLOB_RECURSE integration_sources
    "${PROJECT_SOURCE_DIR}/test/integration/*.c"
    "${PROJECT_SOURCE_DIR}/test/integration/*.h")
foreach(source IN LISTS integration_sources)
    file(READ "${source}" text)
    if(text MATCHES "CopyFileA|session-floppy-[^\"]*\\.img|session-fixed-disk-[^\"]*\\.img|open_with_media_transform")
        message(FATAL_ERROR
            "T533 integration must use the INI-declared VM media overlay, not copied media: ${source}")
    endif()
    # This is the one integration provider that resolves the production INI.
    # into the immutable request used to create the test machine. Every other
    # integration source is a consumer and must not construct a machine.
    if(source MATCHES "/support/session_ini\\.c$" OR
       source MATCHES "\\\\support\\\\session_ini\\.c$")
        continue()
    endif()
    if(text MATCHES "vm_machine_create[ \t\r\n]*\\(")
        message(FATAL_ERROR "T533 integration must open only through the INI session owner: ${source}")
    endif()
    if(text MATCHES "vm_machine_config[ \t\r\n]")
        message(FATAL_ERROR "T533 integration must not construct a session config: ${source}")
    endif()
    if(source MATCHES "/support/.*\\.c$" OR source MATCHES "\\\\support\\\\.*\\.c$")
        continue()
    endif()
    if(source MATCHES "\\.c$" AND NOT text MATCHES
        "integration_ini_session_|vm_app_ini_load|nxvm_console_process_run")
        message(FATAL_ERROR "T533 integration lacks an INI session consumer: ${source}")
    endif()
endforeach()
file(GLOB_RECURSE unit_sources
    "${PROJECT_SOURCE_DIR}/test/app/*.c"
    "${PROJECT_SOURCE_DIR}/test/app/*.h"
    "${PROJECT_SOURCE_DIR}/test/core/machine/*.c"
    "${PROJECT_SOURCE_DIR}/test/core/machine/*.h"
    "${PROJECT_SOURCE_DIR}/test/core/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/test/core/profiles/*.h")
foreach(source IN LISTS unit_sources)
    file(READ "${source}" text)
    if(text MATCHES "vm_machine_create[ \t\r\n]*\\(")
        message(FATAL_ERROR
        "T533 repository-only unit test must use in-memory assets: ${source}")
    endif()
    if(text MATCHES "vm_profile_byob_blob_load|GetFileAttributesA|CopyFileA|CreateFile")
        message(FATAL_ERROR
            "T533 repository-only unit test must not load an external asset: ${source}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_text)
if(cmake_text MATCHES "project_add_test\\([^\\n]*integration[^\\n]*(PROJECT_FDD_SMOKE_IMAGE|PROJECT_M1_FDD_SMOKE_IMAGE|PROJECT_HDD_SMOKE_IMAGE)")
    message(FATAL_ERROR
        "T533 integration must be registered through the INI helper, not a media path.")
endif()
file(GLOB session_documents "${PROJECT_SOURCE_DIR}/assets/sessions/*.ini")
set(expected_session_documents
    "${PROJECT_SOURCE_DIR}/assets/sessions/ibm-5160-model-268-360k.ini"
    "${PROJECT_SOURCE_DIR}/assets/sessions/ibm-5170-model-339-1200k.ini"
    "${PROJECT_SOURCE_DIR}/assets/sessions/compaq-deskpro-386-model-40-1200k.ini"
    "${PROJECT_SOURCE_DIR}/assets/sessions/default-pc-at-80386-1440k-hdd.ini")
list(LENGTH session_documents session_document_count)
if(NOT session_document_count EQUAL 4)
    message(FATAL_ERROR
        "T533 must retain exactly one canonical INI per supported machine.")
endif()
foreach(expected_session_document IN LISTS expected_session_documents)
    if(NOT EXISTS "${expected_session_document}")
        message(FATAL_ERROR
            "T533 canonical INI is missing: ${expected_session_document}")
    endif()
endforeach()
file(GLOB legacy_session_documents "${PROJECT_SOURCE_DIR}/assets/sessions/*.yaml")
if(NOT legacy_session_documents STREQUAL "")
    message(FATAL_ERROR "T533 must not retain YAML product-session variants.")
endif()
foreach(session_document IN LISTS session_documents)
    file(READ "${session_document}" text)
    if(text MATCHES "(bios|video|cmos|font|profile|cpu|firmware)[ 	]*=")
        message(FATAL_ERROR
            "T533 INI must not select firmware or a machine profile: ${session_document}")
    endif()
    if(NOT text MATCHES "\\[media\\]")
        message(FATAL_ERROR "T533 INI must have a media section: ${session_document}")
    endif()
endforeach()
message(STATUS "M5:T533:S4:INTEGRATION-INI-BOUNDARY:OK")
