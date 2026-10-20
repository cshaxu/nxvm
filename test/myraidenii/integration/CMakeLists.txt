add_executable(myraidenii-selected-set-startup-probe selected_set_startup_probe.c)
target_link_libraries(myraidenii-selected-set-startup-probe PRIVATE myraidenii-machine)
if(MSVC)
    target_compile_options(myraidenii-selected-set-startup-probe PRIVATE /UNDEBUG /W4 /WX)
else()
    target_compile_options(myraidenii-selected-set-startup-probe PRIVATE
        -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
endif()

# Owner-ROM execution is opt-in.  It is not a unit or default CTest case.
add_custom_target(myraidenii-selected-set-startup-probe-build
    DEPENDS myraidenii-selected-set-startup-probe)
