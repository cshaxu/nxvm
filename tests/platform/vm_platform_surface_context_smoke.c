#include "type.h"



#include "vm/platform/win32/w32adisp.h"

#include "vm/platform/win32/w32cdisp.h"

C_INT main(C_VOID)
{
    w32cdisp_context *console_a = w32cdisp_context_create();
    w32cdisp_context *console_b = w32cdisp_context_create();
    w32adisp_context *window_a = w32adisp_context_create();
    w32adisp_context *window_b = w32adisp_context_create();

    if (console_a == STD_NULL || console_b == STD_NULL || window_a == STD_NULL ||
        window_b == STD_NULL || console_a == console_b || window_a == window_b ||
        w32cdisp_context_generation(console_a) != 0u ||
        w32cdisp_context_generation(console_b) != 0u ||
        w32adisp_context_generation(window_a) != 0u ||
        w32adisp_context_generation(window_b) != 0u) {
        return 1;
    }
    w32cdisp_context_destroy(console_a);
    w32cdisp_context_destroy(console_b);
    w32adisp_context_destroy(window_a);
    w32adisp_context_destroy(window_b);
    puts("M5:T90:S1:SURFACE-CONTEXT:OK");
    return 0;
}
