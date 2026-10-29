#include "app-mypc110/product_vdisplay.h"

static emulator_machine_state actual_state;

emulator_machine_state emulator_machine_state_get(const emulator_machine *machine)
{
    (void)machine;
    return actual_state;
}

int main(void)
{
    mypc110_vdisplay display;
    emulator_session_command_result result;
    emulator_machine *machine = (emulator_machine *)&display;

    mypc110_vdisplay_initialize(&display);
    actual_state = EMULATOR_MACHINE_STOPPED;
    if (lib_atomic_i32_load_explicit(&display.requested, LIB_MEMORY_ORDER_SEQ_CST) != MYPC110_DISPLAY_DSTN ||
        mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_STOPPED,
            "speed turbo", &result) ||
        !mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_STOPPED,
            "  vdisplay=tft  ", &result) ||
        lib_text_compare(result.text, "Display: tft (next start).\r\n") != 0 ||
        lib_atomic_i32_load_explicit(&display.requested, LIB_MEMORY_ORDER_SEQ_CST) != MYPC110_DISPLAY_TFT)
        return 1;
    if (!mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_PAUSED,
            "vdisplay=dstn", &result) ||
        lib_text_compare(result.text, "Display can only be changed while stopped.\r\n") != 0 ||
        lib_atomic_i32_load_explicit(&display.requested, LIB_MEMORY_ORDER_SEQ_CST) != MYPC110_DISPLAY_TFT)
        return 2;
    if (!mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_RUNNING,
            "vdisplay=dstn", &result) ||
        !mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_ERROR,
            "vdisplay=dstn", &result) ||
        !mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_RESET_COMPLETED,
            "vdisplay=dstn", &result) ||
        lib_atomic_i32_load_explicit(&display.requested, LIB_MEMORY_ORDER_SEQ_CST) != MYPC110_DISPLAY_TFT)
        return 2;
    actual_state = EMULATOR_MACHINE_STARTING;
    if (!mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_STOPPED,
            "vdisplay=dstn", &result) ||
        lib_atomic_i32_load_explicit(&display.requested, LIB_MEMORY_ORDER_SEQ_CST) != MYPC110_DISPLAY_TFT)
        return 3;
    actual_state = EMULATOR_MACHINE_STOPPED;
    if (!mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_STOPPED,
            "vdisplay=invalid", &result) ||
        lib_text_compare(result.text, "Usage: vdisplay=dstn|tft\r\n") != 0 ||
        lib_atomic_i32_load_explicit(&display.requested, LIB_MEMORY_ORDER_SEQ_CST) != MYPC110_DISPLAY_TFT ||
        !mypc110_vdisplay_submit(&display, machine, EMULATOR_SESSION_MACHINE_STOPPED,
            "vdisplay=dstn", &result) ||
        lib_text_compare(result.text, "Display: dstn (next start).\r\n") != 0 ||
        lib_atomic_i32_load_explicit(&display.requested, LIB_MEMORY_ORDER_SEQ_CST) != MYPC110_DISPLAY_DSTN)
        return 4;
    return 0;
}
