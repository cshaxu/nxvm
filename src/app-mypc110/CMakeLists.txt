# The PC110 board binding remains App-local.  This first target is deliberately
# not a product executable: display, storage, input and firmware boot retain
# their separate evidence receivers.
add_library(mypc110-board STATIC pc110_config.c pc110_firmware.c pc110_machine.c)
target_include_directories(mypc110-board PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(mypc110-board PUBLIC types x86-core-machine x86-core-vl82c420
    x86-nxvm-board-base)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mypc110-board PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
