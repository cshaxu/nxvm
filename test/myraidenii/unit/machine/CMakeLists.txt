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
