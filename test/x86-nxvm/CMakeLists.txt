cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared-test-x86-nxvm LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

get_filename_component(X86_NXVM_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/x86-nxvm" ABSOLUTE)
if(NOT TARGET x86-nxvm-product)
    add_subdirectory("${X86_NXVM_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/x86-nxvm")
endif()
set(SHARED_X86_NXVM_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")
shared_register_test(x86-nxvm product/machine x86-nxvm-product)
shared_register_test(x86-nxvm chips/pit825x/pit_8253 x86-nxvm-chip-pit825x)
shared_register_test(x86-nxvm chips/pit825x/pit_readback x86-nxvm-chip-pit825x)
shared_register_test(x86-nxvm chips/pit825x/pit_waveform x86-nxvm-chip-pit825x)
shared_register_test(x86-nxvm chips/pit825x/pit_contract x86-nxvm-chip-pit825x)
shared_register_test(x86-nxvm chips/rtc146818/rtc_calendar x86-nxvm-chip-rtc146818)
shared_register_test(x86-nxvm chips/rtc146818/rtc_contract x86-nxvm-chip-rtc146818)
shared_register_test(x86-nxvm chips/pic8259/pic_contract x86-nxvm-chip-pic8259)
shared_register_test(x86-nxvm chips/pic8259/pic_commands x86-nxvm-chip-pic8259)
shared_register_test(x86-nxvm chips/dma8237/dma_contract x86-nxvm-chip-dma8237)
shared_register_test(x86-nxvm chips/fdc8272/fdc_contract x86-nxvm-chip-fdc8272)
shared_register_test(x86-nxvm chips/fdc8272/fdc_records x86-nxvm-chip-fdc8272)
shared_register_test(x86-nxvm chips/fdc8272/fdc_causes x86-nxvm-chip-fdc8272)
shared_register_test(x86-nxvm chips/fdc8272/fdc_allocation x86-nxvm-chip-fdc8272)
shared_register_test(x86-nxvm chips/hdc/hdc_contract x86-nxvm-chip-hdc)
shared_register_test(x86-nxvm chips/hdc/hdc_taskfile x86-nxvm-chip-hdc)
shared_register_test(x86-nxvm chips/hdc/hdc_allocation x86-nxvm-chip-hdc)
shared_register_test(x86-nxvm chips/video/video_contract x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/video_inspection x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/video_allocation x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/video_text x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/video_text_status x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/compaq_cecg_contract x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/compaq_cecg_feature_environment x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/compaq_cecg_io_base x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/compaq_cecg_input_status x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/ega_crtc_boundary_port x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/ega_sequencer x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/ega_controller x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/ega_external x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/cga_graphics x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/cga_640 x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/ega_mode10 x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/compaq_ega_personality x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/compaq_cecg_odd_even_page x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/video/ega_planar x86-nxvm-chip-video)
shared_register_test(x86-nxvm chips/ps2mouse/mouse_contract x86-nxvm-chip-ps2mouse)
shared_register_test(x86-nxvm chips/keyboard/keyboard_contract x86-nxvm-chip-keyboard)
shared_register_test(x86-nxvm chips/kbc8042/controller_contract "x86-nxvm-chip-kbc8042;x86-nxvm-chip-keyboard;x86-nxvm-chip-ps2mouse")
shared_register_test(x86-nxvm chips/ppi8255/ppi_contract x86-nxvm-chip-ppi8255)
shared_register_test(x86-nxvm chips/xtkeyboard/xt_keyboard_contract x86-nxvm-chip-xtkeyboard)
shared_register_test(x86-nxvm product/ini x86-nxvm-product)
shared_register_test(x86-nxvm product/factory x86-nxvm-product)
shared_register_test(x86-nxvm product/startup x86-nxvm-product)

set(x86_nxvm_external_test_directories
    "${CMAKE_CURRENT_LIST_DIR}/setup"
    "${CMAKE_CURRENT_LIST_DIR}/integration")
string(REPLACE ";" "\\;" x86_nxvm_external_test_directories_argument
    "${x86_nxvm_external_test_directories}")

add_test(NAME x86-nxvm.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_LIST_DIR}"
    "-DTEST_EXCLUDE_DIRECTORIES=${x86_nxvm_external_test_directories_argument}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_types_boundary.cmake")
add_test(NAME x86-nxvm.test-ownership COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_LIST_DIR}" "-DTEST_LAYER=x86-nxvm"
    "-DTEST_EXCLUDE_DIRECTORIES=${x86_nxvm_external_test_directories_argument}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake")
add_test(NAME x86-nxvm.manifest COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${X86_NXVM_ROOT}" -P "${X86_NXVM_ROOT}/verify_manifest.cmake")
add_test(NAME x86-nxvm.corpus COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${X86_NXVM_ROOT}" -P "${X86_NXVM_ROOT}/verify_corpus.cmake")
add_test(NAME x86-nxvm.verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${X86_NXVM_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/verifier-probe"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_negative.cmake")
add_test(NAME x86-nxvm.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_LIST_DIR}" -P "${CMAKE_CURRENT_LIST_DIR}/../verify_manifest.cmake")
set_tests_properties(
    x86-nxvm.test-types-boundary
    x86-nxvm.test-ownership
    x86-nxvm.manifest
    x86-nxvm.corpus
    x86-nxvm.verifier-negative
    x86-nxvm.test-manifest
    PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
# Whole-corpus negative probes need the existing budget without competing
# filesystem copies/compiler self-tests; this does not extend their deadline.
set_tests_properties(x86-nxvm.verifier-negative PROPERTIES RUN_SERIAL TRUE)

add_custom_target(x86-nxvm-product-tests DEPENDS ${SHARED_X86_NXVM_TEST_TARGETS})
set(SHARED_X86_NXVM_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/board_tests.cmake")
add_custom_target(shared-x86-nxvm-tests DEPENDS x86-nxvm-product-tests
    x86-nxvm-board-tests)
