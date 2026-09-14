add_library(common-ui
    ui.c)
target_include_directories(common-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(common-ui PUBLIC
    console host kvm-base kvm-window kvm-console)
