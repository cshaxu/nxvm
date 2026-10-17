cmake_minimum_required(VERSION 3.23)
project(myraidenii-tests LANGUAGES NONE)
enable_testing()
find_program(MYRAIDENII_POWERSHELL NAMES powershell pwsh REQUIRED)
add_test(NAME myraidenii.unit.tools.rom-inventory
    COMMAND "${MYRAIDENII_POWERSHELL}" -NoProfile -ExecutionPolicy Bypass
        -File "${CMAKE_CURRENT_LIST_DIR}/unit/tools/rom_inventory_smoke.ps1"
        -Verifier "${CMAKE_CURRENT_LIST_DIR}/../../tools/myraidenii/Verify-RomInventory.ps1"
        -TemporaryRoot "${CMAKE_CURRENT_BINARY_DIR}/rom-inventory")
set_tests_properties(myraidenii.unit.tools.rom-inventory PROPERTIES
    LABELS "unit;myraidenii" TIMEOUT 60)
