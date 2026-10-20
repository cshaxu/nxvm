add_executable(myraidenii-unit-machine-audio-smoke
    audio_smoke.c
    "${CMAKE_CURRENT_LIST_DIR}/../../../../src/myraidenii/machine/audio.c")
target_include_directories(myraidenii-unit-machine-audio-smoke PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../../..")
target_link_libraries(myraidenii-unit-machine-audio-smoke PRIVATE types)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(myraidenii-unit-machine-audio-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(myraidenii-unit-machine-audio-smoke PRIVATE /W4 /WX)
endif()
add_test(NAME myraidenii.unit.machine.audio
    COMMAND myraidenii-unit-machine-audio-smoke)
set_tests_properties(myraidenii.unit.machine.audio PROPERTIES LABELS "unit;myraidenii" TIMEOUT 30)

add_executable(myraidenii-unit-machine-driver-smoke
    driver_smoke.c
    "${CMAKE_CURRENT_LIST_DIR}/../../../../src/myraidenii/machine/audio.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../../src/myraidenii/machine/driver.c")
target_include_directories(myraidenii-unit-machine-driver-smoke PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../../..")
target_link_libraries(myraidenii-unit-machine-driver-smoke PRIVATE types)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(myraidenii-unit-machine-driver-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(myraidenii-unit-machine-driver-smoke PRIVATE /W4 /WX)
endif()
add_test(NAME myraidenii.unit.machine.driver
    COMMAND myraidenii-unit-machine-driver-smoke)
set_tests_properties(myraidenii.unit.machine.driver PROPERTIES LABELS "unit;myraidenii" TIMEOUT 30)

add_executable(myraidenii-unit-machine-image-hash-smoke image_hash_smoke.c
    "${CMAKE_CURRENT_LIST_DIR}/../../../../src/myraidenii/machine/image_hash.c")
target_include_directories(myraidenii-unit-machine-image-hash-smoke PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../../..")
target_link_libraries(myraidenii-unit-machine-image-hash-smoke PRIVATE types)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(myraidenii-unit-machine-image-hash-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(myraidenii-unit-machine-image-hash-smoke PRIVATE /UNDEBUG /W4 /WX)
endif()
add_test(NAME myraidenii.unit.machine.image-hash COMMAND myraidenii-unit-machine-image-hash-smoke)
set_tests_properties(myraidenii.unit.machine.image-hash PROPERTIES LABELS "unit;myraidenii" TIMEOUT 30)

add_executable(myraidenii-unit-machine-configuration-smoke configuration_smoke.c
    "${CMAKE_CURRENT_LIST_DIR}/../../../../src/myraidenii/machine/configuration.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../../src/myraidenii/machine/image_hash.c")
target_include_directories(myraidenii-unit-machine-configuration-smoke PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../../..")
target_link_libraries(myraidenii-unit-machine-configuration-smoke PRIVATE types)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(myraidenii-unit-machine-configuration-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(myraidenii-unit-machine-configuration-smoke PRIVATE /UNDEBUG /W4 /WX)
endif()
add_test(NAME myraidenii.unit.machine.configuration COMMAND myraidenii-unit-machine-configuration-smoke)
set_tests_properties(myraidenii.unit.machine.configuration PROPERTIES LABELS "unit;myraidenii" TIMEOUT 30)
