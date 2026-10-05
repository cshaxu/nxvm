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
get_property(nxvm_supported_profiles CACHE NXVM_PRODUCT_PROFILE PROPERTY STRINGS)
if(NOT NXVM_PRODUCT_PROFILE IN_LIST nxvm_supported_profiles)
    message(FATAL_ERROR "Unsupported NXVM_PRODUCT_PROFILE: ${NXVM_PRODUCT_PROFILE}")
endif()

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

include("${CMAKE_SOURCE_DIR}/src/app-nxvm/CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/src/app-my5160/CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/src/app-my5170/CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/src/app-mydeskpro386/CMakeLists.txt")

if(NOT NXVM_PRODUCT_MACHINE_KEY)
    message(FATAL_ERROR "Unsupported NXVM_PRODUCT_PROFILE: ${NXVM_PRODUCT_PROFILE}")
endif()

include("${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_my5160_boundary.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_my5170_boundary.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_mydeskpro386_boundary.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_nxvm_app_boundary.cmake")

get_filename_component(nxvm_product_directory "${NXVM_PRODUCT_ENTRY}" DIRECTORY)
get_filename_component(nxvm_app_directory "${nxvm_product_directory}" DIRECTORY)
get_filename_component(nxvm_generated_product "${nxvm_app_directory}" NAME)

# Only products and external integration link this target; unit fixtures do not.
ibmpc_embed_firmware(nxvm-product-firmware
    "${CMAKE_BINARY_DIR}/generated/${nxvm_generated_product}/product/firmware.c"
    "${NXVM_PROFILE_BIOS_0}" "${NXVM_PROFILE_BIOS_1}" "${NXVM_PROFILE_VIDEO}"
    "${NXVM_PROFILE_CMOS}" "${NXVM_PROFILE_FONT}")
