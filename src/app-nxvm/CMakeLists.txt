# NXVM owns its fixed default 386 composition and project-authored firmware.
set(NXVM_COMPOSITION_SOURCES
    src/app-nxvm/machine_plan.c
    src/app-nxvm/machine_config.c)
add_library(nxvm-machine STATIC ${NXVM_COMPOSITION_SOURCES})
target_include_directories(nxvm-machine PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-machine PUBLIC x86-nxvm-machine x86-nxvm-board-base x86-core-machine)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(nxvm-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(NXVM_PRODUCT_PROFILE STREQUAL "default-pc-at-80386-1440k-hdd")
    set(NXVM_PRODUCT_MACHINE_KEY default)
    nxvm_require_manifest(default-pc-at)
    set(NXVM_PROFILE_BIOS_0 "${nxvm_default_firmware_rom}")
    set(NXVM_PROFILE_CMOS "${NXVM_PROFILE_ASSETS_ROOT}/default-pc-at/cmos/default-pc-at-1440k-hdd.cmos")
    set(NXVM_PROFILE_BIOS_COUNT 1)
    set(NXVM_PROFILE_BIOS_1 LIB_NULL)
    set(NXVM_PROFILE_VIDEO LIB_NULL)
    nxvm_require_profile_asset("default-pc-at/cmos/default-pc-at-1440k-hdd.cmos" 64
        "6b02de39b0dbd9ce2516c94545b89b4d1883df4cf6714876c60bef3fd6f93d0d")
    set(NXVM_PRODUCT_ENTRY "${CMAKE_SOURCE_DIR}/src/app-nxvm/main.c")
    set(NXVM_PRODUCT_ARTIFACT_NAME nxvm)
    set(NXVM_PRODUCT_INI_NAME nxvm.ini)
    set(NXVM_PRODUCT_BINDING src/app-nxvm/product_binding.c)
    set(NXVM_PRODUCT_BINDING_HEADER app-nxvm/product_binding_interface.h)
    set(NXVM_PRODUCT_ARTIFACT_ROOT "${CMAKE_SOURCE_DIR}/assets/nxvm")
    set(NXVM_PRODUCT_ARTIFACT_DIRECTORY "${NXVM_PRODUCT_ARTIFACT_ROOT}")
endif()
