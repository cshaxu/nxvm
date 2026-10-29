cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(mypc110-tests LANGUAGES C)
    enable_testing()
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../src/lib" lib)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../src/x86-core" x86-core)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../src/x86-nxvm" x86-nxvm)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../src/app-mypc110" mypc110)
endif()

if(TARGET mypc110-selectable-firmware)
    add_executable(mypc110-firmware-variants-smoke integration/firmware_variants_smoke.c)
    target_include_directories(mypc110-firmware-variants-smoke PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(mypc110-firmware-variants-smoke PRIVATE mypc110-selectable-firmware mypc110-board)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mypc110-firmware-variants-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
    endif()
    if(TARGET build-integration-tests)
        add_dependencies(build-integration-tests mypc110-firmware-variants-smoke)
    endif()
    add_test(NAME mypc110.integration/firmware-variants COMMAND mypc110-firmware-variants-smoke)
    set_tests_properties(mypc110.integration/firmware-variants PROPERTIES LABELS "integration;mypc110" TIMEOUT 30)
endif()

add_executable(mypc110-vdisplay-smoke unit/vdisplay_smoke.c
    "${CMAKE_SOURCE_DIR}/src/app-mypc110/product_vdisplay.c")
target_include_directories(mypc110-vdisplay-smoke PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(mypc110-vdisplay-smoke PRIVATE types)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-vdisplay-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-vdisplay-smoke)
endif()
add_test(NAME mypc110.unit/vdisplay COMMAND mypc110-vdisplay-smoke)
set_tests_properties(mypc110.unit/vdisplay PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

# Reuse the same firmware diagnostic for the separately embedded DSTN BIOS.
# It is not a qualification test until that firmware's checkpoints are proven.
if(TARGET mypc110-real-firmware-dstn)
    add_executable(mypc110-dstn-firmware-diagnostic EXCLUDE_FROM_ALL
        integration/real_firmware_post_integration.c)
    target_link_libraries(mypc110-dstn-firmware-diagnostic PRIVATE
        mypc110-board mypc110-real-firmware-dstn)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mypc110-dstn-firmware-diagnostic PRIVATE
            -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

add_executable(mypc110-config-route-smoke unit/config_route_smoke.c)
target_link_libraries(mypc110-config-route-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-config-route-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-config-route-smoke)
endif()
add_test(NAME mypc110.unit/config-route COMMAND mypc110-config-route-smoke)
set_tests_properties(mypc110.unit/config-route PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

add_executable(mypc110-firmware-binding-smoke unit/firmware_binding_smoke.c)
target_link_libraries(mypc110-firmware-binding-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-firmware-binding-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-firmware-binding-smoke)
endif()
add_test(NAME mypc110.unit/firmware-binding COMMAND mypc110-firmware-binding-smoke)
set_tests_properties(mypc110.unit/firmware-binding PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

add_executable(mypc110-font-rom-smoke unit/font_rom_smoke.c)
target_link_libraries(mypc110-font-rom-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-font-rom-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-font-rom-smoke)
endif()
add_test(NAME mypc110.unit/font-rom COMMAND mypc110-font-rom-smoke)
set_tests_properties(mypc110.unit/font-rom PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

add_executable(mypc110-machine-construction-smoke unit/machine_construction_smoke.c)
target_link_libraries(mypc110-machine-construction-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-machine-construction-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-machine-construction-smoke)
endif()
add_test(NAME mypc110.unit/machine-construction COMMAND mypc110-machine-construction-smoke)
set_tests_properties(mypc110.unit/machine-construction PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

add_executable(mypc110-lpt-smoke unit/lpt_smoke.c)
target_link_libraries(mypc110-lpt-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-lpt-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-lpt-smoke)
endif()
add_test(NAME mypc110.unit/lpt COMMAND mypc110-lpt-smoke)
set_tests_properties(mypc110.unit/lpt PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

add_executable(mypc110-inking-smoke unit/inking_smoke.c)
target_link_libraries(mypc110-inking-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-inking-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-inking-smoke)
endif()
add_test(NAME mypc110.unit/inking COMMAND mypc110-inking-smoke)
set_tests_properties(mypc110.unit/inking PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

add_executable(mypc110-pcic-smoke unit/pc110_pcic_smoke.c)
target_link_libraries(mypc110-pcic-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-pcic-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-pcic-smoke)
endif()
add_test(NAME mypc110.unit/pcic COMMAND mypc110-pcic-smoke)
set_tests_properties(mypc110.unit/pcic PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

add_executable(mypc110-ec-mailbox-smoke unit/ec_mailbox_smoke.c)
target_link_libraries(mypc110-ec-mailbox-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-ec-mailbox-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-ec-mailbox-smoke)
endif()
add_test(NAME mypc110.unit/ec-mailbox COMMAND mypc110-ec-mailbox-smoke)
set_tests_properties(mypc110.unit/ec-mailbox PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

add_executable(mypc110-ec-block-smoke unit/ec_block_smoke.c)
target_link_libraries(mypc110-ec-block-smoke PRIVATE mypc110-board)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-ec-block-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()
if(TARGET build-unit-tests)
    add_dependencies(build-unit-tests mypc110-ec-block-smoke)
endif()
add_test(NAME mypc110.unit/ec-block COMMAND mypc110-ec-block-smoke)
set_tests_properties(mypc110.unit/ec-block PROPERTIES LABELS "unit;mypc110" TIMEOUT 30)

if(TARGET mypc110-product)
    add_executable(mypc110-product-composition-smoke integration/product_composition_smoke.c)
    target_link_libraries(mypc110-product-composition-smoke PRIVATE mypc110-product)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mypc110-product-composition-smoke PRIVATE
            -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
    endif()
    if(TARGET build-integration-tests)
        add_dependencies(build-integration-tests mypc110-product-composition-smoke)
    endif()
    add_test(NAME mypc110.integration/product-composition COMMAND mypc110-product-composition-smoke)
    set_tests_properties(mypc110.integration/product-composition PROPERTIES
        LABELS "integration;mypc110" TIMEOUT 30)
endif()

if(WIN32 AND TARGET mypc110)
    add_executable(mypc110-cmos-product-smoke
        integration/cmos_product_smoke.c "${CMAKE_SOURCE_DIR}/test/setup/console_process.c")
    target_include_directories(mypc110-cmos-product-smoke PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mypc110-cmos-product-smoke PRIVATE storage)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mypc110-cmos-product-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    foreach(variant IN ITEMS tft dstn)
        add_test(NAME mypc110.integration/${variant}-cmos-product
            COMMAND mypc110-cmos-product-smoke $<TARGET_FILE:mypc110>
                ${variant} "${CMAKE_BINARY_DIR}")
        set_tests_properties(mypc110.integration/${variant}-cmos-product PROPERTIES
            LABELS "integration;mypc110" TIMEOUT 30 RUN_SERIAL TRUE)
        if(CMAKE_BUILD_TYPE STREQUAL "Release" AND variant STREQUAL "dstn")
            get_target_property(artifact mypc110 MYPC110_ARTIFACT_NAME)
            get_target_property(bios mypc110 MYPC110_DSTN_BIOS_PATH)
            get_target_property(other mypc110 MYPC110_TFT_BIOS_PATH)
            get_target_property(font mypc110 MYPC110_FONT_PATH)
            add_test(NAME mypc110.integration/artifact-firmware
                COMMAND "${CMAKE_COMMAND}"
                    "-DPROJECT_ARTIFACT_PATH=$<TARGET_FILE:mypc110>"
                    "-DPROJECT_ARTIFACT_ARCHITECTURE=${PROJECT_ARTIFACT_ARCHITECTURE}"
                    "-DDEPLOYED_PATH=${CMAKE_SOURCE_DIR}/assets/mypc110/${artifact}"
                    "-DEXPECTED_BIOS=${bios}"
                    "-DOTHER_BIOS=${other}"
                    "-DFONT_FILE=${font}"
                    "-DSOURCE_ROOT=${CMAKE_SOURCE_DIR}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/integration/artifact_firmware.cmake")
            set_tests_properties(mypc110.integration/artifact-firmware PROPERTIES
                LABELS "integration;mypc110" TIMEOUT 120)
            add_test(NAME mypc110.integration/artifact-wrong-firmware
                COMMAND "${CMAKE_COMMAND}"
                    "-DPROJECT_ARTIFACT_PATH=$<TARGET_FILE:mypc110>"
                    "-DPROJECT_ARTIFACT_ARCHITECTURE=${PROJECT_ARTIFACT_ARCHITECTURE}"
                    "-DDEPLOYED_PATH=${CMAKE_SOURCE_DIR}/assets/mypc110/${artifact}"
                    "-DEXPECTED_BIOS=${other}"
                    "-DOTHER_BIOS=${bios}"
                    "-DFONT_FILE=${font}"
                    "-DSOURCE_ROOT=${CMAKE_SOURCE_DIR}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/integration/artifact_firmware_negative.cmake")
            set_tests_properties(mypc110.integration/artifact-wrong-firmware PROPERTIES
                LABELS "integration;mypc110" TIMEOUT 120)
        endif()
    endforeach()
endif()

if(TARGET mypc110-real-firmware-tft)
    add_executable(mypc110-real-firmware-post-integration
        integration/real_firmware_post_integration.c)
    target_link_libraries(mypc110-real-firmware-post-integration PRIVATE
        mypc110-board mypc110-real-firmware-tft)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mypc110-real-firmware-post-integration PRIVATE
            -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
    endif()
    # The firmware receiver covers the earlier timer/SMI prefixes and
    # the integrated controller's reset, IRQ and four SENSE results.
    add_test(NAME mypc110.integration/real-firmware-fdc-reset
        COMMAND mypc110-real-firmware-post-integration --fdc-reset)
    set_tests_properties(mypc110.integration/real-firmware-fdc-reset PROPERTIES
        LABELS "integration;mypc110" TIMEOUT 180)
    # Original BIOS branch: retained power plus ordinary POST/INT19.
    # Keep this CPU-bound whole route isolated from other CTest workloads.
    add_test(NAME mypc110.integration/real-firmware-resume-boot-handoff
        COMMAND mypc110-real-firmware-post-integration --resume-boot)
    # The optimized x86 guest handoff measured 356 seconds; retain bounded
    # host headroom without changing the guest or the x64 budget.
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(mypc110_boot_timeout 600)
    else()
        set(mypc110_boot_timeout 360)
    endif()
    set_tests_properties(mypc110.integration/real-firmware-resume-boot-handoff PROPERTIES
        LABELS "integration;mypc110" TIMEOUT ${mypc110_boot_timeout} RUN_SERIAL TRUE)
    add_test(NAME mypc110.integration/real-firmware-cold-boot-handoff
        COMMAND mypc110-real-firmware-post-integration --cold-boot)
    set_tests_properties(mypc110.integration/real-firmware-cold-boot-handoff PROPERTIES
        LABELS "integration;mypc110" TIMEOUT ${mypc110_boot_timeout} RUN_SERIAL TRUE)
endif()
