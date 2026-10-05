# One product executable has one immutable board selection.  This file is the
# only CMake selection table; NXVM.ini deliberately has no profile/firmware key.
set(NXVM_PROFILE_ASSETS_ROOT "${CMAKE_SOURCE_DIR}/../nxvm-assets/profiles-nxvm" CACHE PATH
    "BYOB profile archive containing profile manifests and firmware")
set(NXVM_PRODUCT_PROFILE "default-pc-at-80386-1440k-hdd" CACHE STRING
    "One fixed NXVM product profile")
set_property(CACHE NXVM_PRODUCT_PROFILE PROPERTY STRINGS
    default-pc-at-80386-1440k-hdd ibm-5160-model-268-360k
    ibm-5170-model-339-1200k
    compaq-deskpro-386-model-40-1200k)

if(NOT EXISTS "${NXVM_PROFILE_ASSETS_ROOT}")
    message(FATAL_ERROR "NXVM_PROFILE_ASSETS_ROOT must name a BYOB profiles-nxvm archive")
endif()

function(nxvm_require_profile_asset relative bytes sha256)
    set(path "${NXVM_PROFILE_ASSETS_ROOT}/${relative}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Required BYOB profile asset is missing: ${path}")
    endif()
    file(SIZE "${path}" actual_bytes)
    file(SHA256 "${path}" actual_sha256)
    if(NOT actual_bytes EQUAL bytes OR NOT actual_sha256 STREQUAL sha256)
        message(FATAL_ERROR "BYOB profile asset does not match its manifest: ${path}")
    endif()
endfunction()

function(nxvm_require_manifest profile)
    set(path "${NXVM_PROFILE_ASSETS_ROOT}/${profile}/manifest.yaml")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Required BYOB profile manifest is missing: ${path}")
    endif()
    file(READ "${path}" text)
    string(FIND "${text}" "schema: nxvm-asset-manifest" schema_index)
    string(FIND "${text}" "profile: ${profile}" profile_index)
    if(schema_index EQUAL -1 OR profile_index EQUAL -1)
        message(FATAL_ERROR "Invalid BYOB profile manifest: ${path}")
    endif()
endfunction()

set(NXVM_PROFILE_FONT "${NXVM_PROFILE_ASSETS_ROOT}/ibm-mda/firmware/ibm-mda-cp437-character-generator.rom")
nxvm_require_manifest(ibm-mda)
nxvm_require_profile_asset("ibm-mda/firmware/ibm-mda-cp437-character-generator.rom" 8192
    "37527f661580e5a09710051cec67422abfadf31c61b841537a91e7f27be19304")

set(NXVM_PRODUCT_ENTRY "${CMAKE_SOURCE_DIR}/src/app-nxvm/product/main.c")
set(NXVM_PRODUCT_BINDING src/app-nxvm/product/machine_binding.c)
set(NXVM_PRODUCT_BINDING_HEADER app-nxvm/product/profile_binding.h)
set(NXVM_PRODUCT_ARTIFACT_ROOT "${CMAKE_SOURCE_DIR}/assets/nxvm")
set(NXVM_PRODUCT_ARTIFACT_DIRECTORY "${NXVM_PRODUCT_ARTIFACT_ROOT}/${NXVM_PRODUCT_PROFILE}")
include("${CMAKE_SOURCE_DIR}/src/app-my5160/CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/src/app-my5170/CMakeLists.txt")

if(NXVM_PRODUCT_PROFILE STREQUAL "default-pc-at-80386-1440k-hdd")
    set(NXVM_PRODUCT_MACHINE_KEY default)
    set(NXVM_PROFILE_MONITOR_NAME "default-pc-at")
    set(NXVM_PROFILE_PLAN_CREATE vm_profile_machine_plan_create_default)
    set(NXVM_PROFILE_CONSTRUCTION_HEADER app-nxvm/profiles/default_profile/construction_interface.h)
    set(NXVM_PROFILE_CPU CORE_MACHINE_CPU_PROFILE_80386)
    set(NXVM_PROFILE_FPU X86_FPU_PROFILE_NONE)
    set(NXVM_PROFILE_FLOPPY_FORMAT VM_MACHINE_FLOPPY_FORMAT_1440K)
    set(product_asset_key 1440k-hdd)
    nxvm_require_manifest(default-pc-at)
    set(NXVM_PROFILE_BIOS_0 "${nxvm_default_firmware_rom}")
    set(NXVM_PROFILE_CMOS "${NXVM_PROFILE_ASSETS_ROOT}/default-pc-at/cmos/default-pc-at-${product_asset_key}.cmos")
    set(NXVM_PROFILE_BIOS_COUNT 1)
    set(NXVM_PROFILE_BIOS_1 LIB_NULL)
    set(NXVM_PROFILE_VIDEO LIB_NULL)
    set(cmos_hash 6b02de39b0dbd9ce2516c94545b89b4d1883df4cf6714876c60bef3fd6f93d0d)
    nxvm_require_profile_asset("default-pc-at/cmos/default-pc-at-${product_asset_key}.cmos" 64 "${cmos_hash}")
elseif(NXVM_PRODUCT_PROFILE STREQUAL "ibm-5160-model-268-360k")
    # My5160's own entry supplied the immutable XT selection and assets above.
elseif(NXVM_PRODUCT_PROFILE STREQUAL "ibm-5170-model-339-1200k")
    # My5170 supplies its own fixed selection and assets above.
elseif(NXVM_PRODUCT_PROFILE STREQUAL "compaq-deskpro-386-model-40-1200k")
    set(NXVM_PRODUCT_MACHINE_KEY model40)
    set(NXVM_PROFILE_MONITOR_NAME "compaq-deskpro-386-model-40")
    set(NXVM_PROFILE_PLAN_CREATE vm_profile_machine_plan_create_model40)
    set(NXVM_PROFILE_CONSTRUCTION_HEADER app-nxvm/profiles/model40/construction_interface.h)
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
else()
    message(FATAL_ERROR "Unsupported NXVM_PRODUCT_PROFILE: ${NXVM_PRODUCT_PROFILE}")
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/app-nxvm/product")
include("${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_my5160_boundary.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_my5170_boundary.cmake")
configure_file("${CMAKE_SOURCE_DIR}/cmake/nxvm/profile_binding.h.in"
    "${CMAKE_BINARY_DIR}/generated/app-nxvm/product/profile_binding.h" @ONLY)

if(NXVM_PRODUCT_MACHINE_KEY STREQUAL "xt")
    set(nxvm_generated_product app-my5160)
elseif(NXVM_PRODUCT_MACHINE_KEY STREQUAL "at")
    set(nxvm_generated_product app-my5170)
else()
    set(nxvm_generated_product app-nxvm)
endif()

# Only products and external integration link this target; unit fixtures do not.
ibmpc_embed_firmware(nxvm-product-firmware
    "${CMAKE_BINARY_DIR}/generated/${nxvm_generated_product}/product/firmware.c"
    "${NXVM_PROFILE_BIOS_0}" "${NXVM_PROFILE_BIOS_1}" "${NXVM_PROFILE_VIDEO}"
    "${NXVM_PROFILE_CMOS}" "${NXVM_PROFILE_FONT}")
