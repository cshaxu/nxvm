# My5160 owns only its fixed XT composition and App binding. The family tools
# configure this entry; the production graph never links another PC App.
set(MY5160_MACHINE_SOURCES
    src/app-my5160/machine_config.c
    src/app-my5160/firmware_map.c
    src/app-my5160/machine_plan.c)
add_library(my5160-machine STATIC ${MY5160_MACHINE_SOURCES})
target_include_directories(my5160-machine PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(my5160-machine PUBLIC core-machine core-board-base core-x86)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(my5160-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(NXVM_PRODUCT_PROFILE STREQUAL "ibm-5160-model-268-360k")
    set(NXVM_PRODUCT_MACHINE_KEY xt)
    set(NXVM_PROFILE_BIOS_COUNT 1)
    set(NXVM_PROFILE_BIOS_0 "${NXVM_PROFILE_ASSETS_ROOT}/ibm-5160/firmware/ibm-5160-16aug82-logical-f0000-fffff.rom")
    set(NXVM_PROFILE_BIOS_1 LIB_NULL)
    set(NXVM_PROFILE_VIDEO LIB_NULL)
    set(NXVM_PROFILE_CMOS LIB_NULL)
    set(NXVM_PRODUCT_ENTRY "${CMAKE_SOURCE_DIR}/src/app-my5160/main.c")
    set(NXVM_PRODUCT_BINDING src/app-my5160/product_binding.c)
    set(NXVM_PRODUCT_BINDING_HEADER app-my5160/product_binding_interface.h)
    set(NXVM_PRODUCT_ARTIFACT_ROOT "${CMAKE_SOURCE_DIR}/assets/my5160")
    set(NXVM_PRODUCT_ARTIFACT_DIRECTORY "${NXVM_PRODUCT_ARTIFACT_ROOT}")
    nxvm_require_manifest(ibm-5160)
    nxvm_require_profile_asset("ibm-5160/firmware/ibm-5160-16aug82-logical-f0000-fffff.rom" 65536 "becff9b23527e20cfb48c7362f315fd099723e9d1e371d2a782a1640a54e1a0a")
endif()
