add_library(emulator-session
    session.c
    control.c
    control_state.c
    presentation_plan.c)
target_include_directories(emulator-session PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(emulator-session PUBLIC kvm-base emulator-machine PRIVATE emulator-ui base)
