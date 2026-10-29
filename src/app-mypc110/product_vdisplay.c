#include "app-mypc110/product_vdisplay.h"
#include "lib/types/file.h"

void mypc110_vdisplay_initialize(mypc110_vdisplay *display)
{
    if (display != LIB_NULL)
        lib_atomic_i32_initialize(&display->requested, MYPC110_DISPLAY_DSTN);
}

lib_bool mypc110_vdisplay_submit(void *context, emulator_machine *machine,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out)
{
    mypc110_vdisplay *display = context;
    const char *end;
    lib_size length;
    mypc110_display_variant selected;

    if (display == LIB_NULL || line == LIB_NULL || out == LIB_NULL) return LIB_FALSE;
    while (*line == ' ' || *line == '\t') ++line;
    end = line + lib_text_length(line);
    while (end > line && (end[-1] == ' ' || end[-1] == '\t')) --end;
    length = (lib_size)(end - line);
    if (length < 8u || lib_memory_compare(line, "vdisplay", 8u) != 0 ||
        (length > 8u && line[8] != '=' && line[8] != ' ' && line[8] != '\t'))
        return LIB_FALSE;
    *out = (emulator_session_command_result){0};
    out->arm_prompt = LIB_TRUE;
    if (length == 13u && lib_memory_compare(line, "vdisplay=dstn", length) == 0)
        selected = MYPC110_DISPLAY_DSTN;
    else if (length == 12u && lib_memory_compare(line, "vdisplay=tft", length) == 0)
        selected = MYPC110_DISPLAY_TFT;
    else {
        (void)lib_c_snprintf(out->text, sizeof(out->text),
            "Usage: vdisplay=dstn|tft\r\n");
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
