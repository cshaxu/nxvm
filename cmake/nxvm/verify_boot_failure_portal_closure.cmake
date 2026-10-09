if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/lifecycle.c" lifecycle)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/pc_at_preparation.c" plan)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/pc_at_rom.c" provider)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/rom_mapping.c" mapping)
file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)

string(FIND "${plan}" "vm_profile_external_pc_at_rom_provider" position)
if(position EQUAL -1)
    message(FATAL_ERROR "External-ROM plan boundary missing its provider")
endif()
string(FIND "${lifecycle}" "core_machine_bind_firmware_provider" position)
if(position EQUAL -1)
    message(FATAL_ERROR "External-ROM lifecycle boundary missing provider binding")
endif()
string(FIND "${provider}" "vm_profile_rom_register" position)
if(position EQUAL -1)
    message(FATAL_ERROR "External-ROM provider bypasses the shared mapping boundary")
endif()
foreach(required "core_machine_firmware_register_immutable_rom("
    "core_machine_firmware_register_immutable_rom_alias(")
    string(FIND "${mapping}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Shared ROM mapping lacks its Core contract: ${required}")
    endif()
endforeach()
foreach(forbidden "default_pc_at.c" "rom/bios.c" "rom/qdcga.c")
    string(FIND "${cmake_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Generated product firmware remains in CMake: ${forbidden}")
    endif()
endforeach()

# Profiles map supplied immutable bytes; only the build may open ROM files.
file(GLOB_RECURSE profile_sources
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/profiles/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/*.h")
list(APPEND profile_sources "${PROJECT_SOURCE_DIR}/src/core/board-base/rom_mapping.c"
    "${PROJECT_SOURCE_DIR}/src/core/board-base/pc_at_rom.c")
foreach(path IN LISTS profile_sources)
    file(READ "${path}" text)
    if(text MATCHES "lib_storage_file_|lib_storage_medium_open|fopen[ \t]*\\(|CreateFile|ReadFile|bios_path|video_path|font_path|create_file_backed|byob_manifest_load")
        message(FATAL_ERROR "Runtime firmware file route in profile: ${path}")
    endif()
endforeach()

message(STATUS "M5:T539:S12:IMMUTABLE-ROM-CLOSURE:OK")
