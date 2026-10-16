if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT EXISTS "${PROJECT_SOURCE_DIR}/test/app-nxvm/integration")
    message(FATAL_ERROR "integration source root is required.")
endif()

file(GLOB_RECURSE integration_sources
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/integration/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/integration/*.h"
    "${PROJECT_SOURCE_DIR}/test/app-my5160/integration/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-my5160/integration/*.h"
    "${PROJECT_SOURCE_DIR}/test/app-my5170/integration/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-my5170/integration/*.h"
    "${PROJECT_SOURCE_DIR}/test/app-mydeskpro386/integration/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-mydeskpro386/integration/*.h")
foreach(source IN LISTS integration_sources)
    file(READ "${source}" text)
    if(text MATCHES "CopyFileA|session-floppy-[^\"]*\\.img|session-fixed-disk-[^\"]*\\.img|open_with_media_transform")
        message(FATAL_ERROR
            "integration must use the INI-declared VM media overlay, not copied media: ${source}")
    endif()
    # This is the one integration provider that resolves the production INI.
    # into the immutable request used to create the test machine. Every other
    # integration source is a consumer and must not construct a machine.
    if(source MATCHES "/support/session_ini\\.c$" OR
       source MATCHES "\\\\support\\\\session_ini\\.c$")
        continue()
    endif()
    if(text MATCHES "x86_nxvm_machine_create[ \t\r\n]*\\(")
        message(FATAL_ERROR "integration must open only through the INI session owner: ${source}")
    endif()
    if(text MATCHES "x86_nxvm_machine_config[ \t\r\n]")
        message(FATAL_ERROR "integration must not construct a session config: ${source}")
    endif()
    if(source MATCHES "/support/.*\\.c$" OR source MATCHES "\\\\support\\\\.*\\.c$")
        continue()
    endif()
    # The deployed-product probe starts the real EXE beside its owner's INI;
    # it must not link or construct an in-process machine/session fixture.
    if(source MATCHES "/product/nxvm_deployed_boot_probe\\.c$")
        if(NOT text MATCHES "CreateProcessA\\(argv\\[1\\]" OR
           NOT text MATCHES "NULL, argv\\[2\\], &startup, &process" OR
           text MATCHES "x86_nxvm_board_create[ \t\r\n]*\\(")
            message(FATAL_ERROR "Deployed probe must launch the real product: ${source}")
        endif()
        continue()
    endif()
    if(source MATCHES "\\.c$" AND NOT text MATCHES
        "integration_ini_session_|x86_nxvm_product_ini_load|pc_console_process_run")
        message(FATAL_ERROR "integration lacks an INI session consumer: ${source}")
    endif()
endforeach()
file(GLOB_RECURSE unit_sources
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/unit/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/unit/*.h"
    "${PROJECT_SOURCE_DIR}/test/x86-nxvm/machine/*.c"
    "${PROJECT_SOURCE_DIR}/test/x86-nxvm/machine/*.h"
    "${PROJECT_SOURCE_DIR}/test/x86-nxvm/board-base/*.c"
    "${PROJECT_SOURCE_DIR}/test/x86-nxvm/board-base/*.h"
    "${PROJECT_SOURCE_DIR}/test/app-my5160/unit/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-my5160/unit/*.h"
    "${PROJECT_SOURCE_DIR}/test/app-my5170/unit/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-my5170/unit/*.h"
    "${PROJECT_SOURCE_DIR}/test/app-mydeskpro386/unit/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-mydeskpro386/unit/*.h")
foreach(source IN LISTS unit_sources)
    file(READ "${source}" text)
    if(text MATCHES "GetFileAttributesA|CopyFileA|CreateFile")
        message(FATAL_ERROR
            "Repository-only unit test must not load an external asset: ${source}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_text)
file(READ "${PROJECT_SOURCE_DIR}/cmake/nxvm/NxvmProduct.cmake" product_cmake_text)
if(product_cmake_text MATCHES "configure_file\\([^)]*NXVM\\.ini" OR
   NOT product_cmake_text MATCHES "set\\(directory \"\\$\\{NXVM_PRODUCT_ARTIFACT_DIRECTORY\\}\"\\)" OR
   NOT product_cmake_text MATCHES "set\\(PROJECT_CURRENT_VM_RUNTIME_PATH \"\\$\\{directory\\}/\\$\\{task_artifact_filename\\}\"" OR
   NOT product_cmake_text MATCHES "project_add_test\\(\\$\\{target\\} integration[ \t\r\n]+\"\\$\\{project_console_runtime_directory\\}\"")
    message(FATAL_ERROR
        "Console integration must launch the deployed EXE beside its owner INI, not a copied build-tree INI.")
endif()
if(cmake_text MATCHES "project_add_test\\([^\\n]*integration[^\\n]*(PROJECT_FDD_SMOKE_IMAGE|PROJECT_M1_FDD_SMOKE_IMAGE|PROJECT_HDD_SMOKE_IMAGE)")
    message(FATAL_ERROR
        "integration must be registered through the INI helper, not a media path.")
endif()
set(session_documents)
foreach(app IN ITEMS my5160 my5170 mydeskpro386 nxvm)
    file(READ "${PROJECT_SOURCE_DIR}/src/app-${app}/CMakeLists.txt" app_build)
    file(READ "${PROJECT_SOURCE_DIR}/src/app-${app}/main.c" app_entry)
    if(NOT app_build MATCHES "set\\(NXVM_PRODUCT_ARTIFACT_NAME ${app}\\)" OR
       NOT app_build MATCHES "set\\(NXVM_PRODUCT_INI_NAME ${app}\\.ini\\)" OR
       NOT app_entry MATCHES "x86_nxvm_product_run\\(APP_PRODUCT_NAME, APP_PRODUCT_INI_NAME,")
        message(FATAL_ERROR "App must explicitly provide its publication/INI identity: ${app}")
    endif()
    file(GLOB app_ini_names RELATIVE "${PROJECT_SOURCE_DIR}/assets/${app}"
        "${PROJECT_SOURCE_DIR}/assets/${app}/*.ini")
    if(NOT app_ini_names STREQUAL "${app}.ini")
        message(FATAL_ERROR "Exactly one lowercase product INI is required: ${app}")
    endif()
    list(APPEND session_documents "${PROJECT_SOURCE_DIR}/assets/${app}/${app}.ini")
endforeach()
file(GLOB legacy_session_documents "${PROJECT_SOURCE_DIR}/assets/nxvm/*/*.yaml")
if(NOT legacy_session_documents STREQUAL "")
    message(FATAL_ERROR "YAML product-session variants must not remain.")
endif()
foreach(session_document IN LISTS session_documents)
    file(READ "${session_document}" text)
    if(text MATCHES "(bios|video|cmos|font|profile|cpu|firmware)[ 	]*=")
        message(FATAL_ERROR
            "INI must not select firmware or a machine profile: ${session_document}")
    endif()
    if(NOT text MATCHES "\\[media\\]")
        message(FATAL_ERROR "INI must have a media section: ${session_document}")
    endif()
endforeach()
message(STATUS "INTEGRATION-INI-BOUNDARY:OK")
