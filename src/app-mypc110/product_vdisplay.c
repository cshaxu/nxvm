#include "app-mypc110/product_vdisplay.h"
#include "lib/types/file.h"

void mypc110_vdisplay_initialize(mypc110_vdisplay *display)
{
    if (display != LIB_NULL)
        lib_atomic_i32_initialize(&display->requested, MYPC110_DISPLAY_DSTN);
}

lib_bool mypc110_vdisplay_submit(void *context, emulator_machine *machine,
    emulator_session_machine_state state, const char *name, const emulator_product_monitor_arguments *arguments,
    emulator_session_command_result *out)
{
    mypc110_vdisplay *display = context;
    mypc110_display_variant selected;

    if (display == LIB_NULL || name == LIB_NULL || arguments == LIB_NULL || out == LIB_NULL)
        return LIB_FALSE;
    if (lib_text_compare(name, "vdisplay") != 0) return LIB_FALSE;
    *out = (emulator_session_command_result){0};
    out->arm_prompt = LIB_TRUE;
    if (arguments->count == 1u && lib_text_compare(arguments->values[0], "dstn") == 0)
        selected = MYPC110_DISPLAY_DSTN;
    else if (arguments->count == 1u && lib_text_compare(arguments->values[0], "tft") == 0)
        selected = MYPC110_DISPLAY_TFT;
    else {
        (void)lib_c_snprintf(out->text, sizeof(out->text),
            "Usage: vdisplay <dstn|tft>\r\n");
        return LIB_TRUE;
    }
    if (machine == LIB_NULL || state != EMULATOR_SESSION_MACHINE_STOPPED ||
        emulator_machine_state_get(machine) != EMULATOR_MACHINE_STOPPED) {
        (void)lib_c_snprintf(out->text, sizeof(out->text),
            "Display can only be changed while stopped.\r\n");
        return LIB_TRUE;
    }
    lib_atomic_i32_store_explicit(&display->requested, selected, LIB_MEMORY_ORDER_SEQ_CST);
    (void)lib_c_snprintf(out->text, sizeof(out->text),
        "Display: %s (next start).\r\n", selected == MYPC110_DISPLAY_TFT ? "tft" : "dstn");
    return LIB_TRUE;
}
