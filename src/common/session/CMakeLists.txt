add_library(common-session
    session.c
    control.c
    control_state.c
    reconciler.c
    presentation_plan.c)
target_include_directories(common-session PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(common-session PUBLIC kvm-base common-machine PRIVATE common-ui host)
