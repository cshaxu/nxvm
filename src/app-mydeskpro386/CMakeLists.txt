# MyDeskPro386 owns its fixed composition and genuine D4 implementation.
set(MYDESKPRO386_D4_SOURCES
    src/app-mydeskpro386/profiles/d4_memory.c
    src/app-mydeskpro386/profiles/d4_platform.c)
set(MYDESKPRO386_COMPOSITION_SOURCES
    src/app-mydeskpro386/profiles/machine_plan.c
    src/app-mydeskpro386/profiles/model40.c
    src/app-mydeskpro386/profiles/composition.c
    src/app-mydeskpro386/profiles/rom/model40_rom.c)
add_library(mydeskpro386-profile STATIC ${MYDESKPRO386_COMPOSITION_SOURCES})
target_include_directories(mydeskpro386-profile PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(mydeskpro386-profile PUBLIC ibmpc-machine ibmpc-board-common x86-core core-machine)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(mydeskpro386-profile PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(NXVM_PRODUCT_PROFILE STREQUAL "compaq-deskpro-386-model-40-1200k")
    set(NXVM_PRODUCT_MACHINE_KEY model40)
    set(NXVM_PROFILE_MONITOR_NAME "compaq-deskpro-386-model-40")
    set(NXVM_PROFILE_PLAN_CREATE vm_profile_machine_plan_create_model40)
    set(NXVM_PROFILE_CONSTRUCTION_HEADER app-mydeskpro386/profiles/construction_interface.h)
    set(NXVM_PROFILE_CPU CORE_MACHINE_CPU_PROFILE_DEFAULT)
    set(NXVM_PROFILE_FPU X86_FPU_PROFILE_NONE)
    set(NXVM_PROFILE_FLOPPY_FORMAT VM_MACHINE_FLOPPY_FORMAT_1200K)
    set(NXVM_PROFILE_BIOS_COUNT 2)
    set(NXVM_PROFILE_BIOS_0 "${NXVM_PROFILE_ASSETS_ROOT}/deskpro-386-model-40/firmware/compaq-deskpro-386-16-rev-e-1986-08-19-even-108285-001.rom")
    set(NXVM_PROFILE_BIOS_1 "${NXVM_PROFILE_ASSETS_ROOT}/deskpro-386-model-40/firmware/compaq-deskpro-386-16-rev-e-1986-08-19-odd-108284-001.rom")
    set(NXVM_PROFILE_VIDEO "${NXVM_PROFILE_ASSETS_ROOT}/deskpro-386-model-40/firmware/compaq-ega-108281-001.rom")
    set(NXVM_PROFILE_CMOS "${NXVM_PROFILE_ASSETS_ROOT}/deskpro-386-model-40/cmos/deskpro-386-model-40-default.cmos")
    nxvm_require_manifest(deskpro-386-model-40)
    nxvm_require_profile_asset("deskpro-386-model-40/firmware/compaq-deskpro-386-16-rev-e-1986-08-19-even-108285-001.rom" 16384 "15fa21fe2b57970f4223dd15996b28983e726f42dc5b508226327f44099f8c41")
    nxvm_require_profile_asset("deskpro-386-model-40/firmware/compaq-deskpro-386-16-rev-e-1986-08-19-odd-108284-001.rom" 16384 "00e5c4f74c7baabc283b996af2cf9d8538cab52b16c8758cb96b136935b42762")
    nxvm_require_profile_asset("deskpro-386-model-40/firmware/compaq-ega-108281-001.rom" 16384 "e3ce21b0b6c23e519d11568710ea59d669ab7167f75ece0dd68157e4508bb0d7")
    nxvm_require_profile_asset("deskpro-386-model-40/cmos/deskpro-386-model-40-default.cmos" 64 "0bae7ec0f94a611fb6e705597870b1871437dc784b63d9b4b2695198ddd43f48")
    set(NXVM_PRODUCT_ENTRY "${CMAKE_SOURCE_DIR}/src/app-mydeskpro386/product/main.c")
    set(NXVM_PRODUCT_BINDING src/app-mydeskpro386/product/machine_binding.c)
    set(NXVM_PRODUCT_BINDING_HEADER app-mydeskpro386/product/binding_interface.h)
    set(NXVM_PRODUCT_ARTIFACT_ROOT "${CMAKE_SOURCE_DIR}/assets/mydeskpro386")
    set(NXVM_PRODUCT_ARTIFACT_DIRECTORY "${NXVM_PRODUCT_ARTIFACT_ROOT}")
endif()
