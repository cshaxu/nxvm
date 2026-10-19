cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(myraidenii-tests LANGUAGES C)
    enable_testing()
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../src/lib" lib)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../src/x86-core" x86-core)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../src/myraidenii" myraidenii)
endif()
find_program(MYRAIDENII_POWERSHELL NAMES powershell pwsh REQUIRED)
add_test(NAME myraidenii.unit.tools.rom-inventory
    COMMAND "${MYRAIDENII_POWERSHELL}" -NoProfile -ExecutionPolicy Bypass
        -File "${CMAKE_CURRENT_LIST_DIR}/unit/tools/rom_inventory_smoke.ps1"
        -Verifier "${CMAKE_CURRENT_LIST_DIR}/../../tools/myraidenii/Verify-RomInventory.ps1"
        -TemporaryRoot "${CMAKE_CURRENT_BINARY_DIR}/rom-inventory")
set_tests_properties(myraidenii.unit.tools.rom-inventory PROPERTIES
    LABELS "unit;myraidenii" TIMEOUT 60)
add_subdirectory(unit/core)
