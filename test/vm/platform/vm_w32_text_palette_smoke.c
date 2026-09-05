#include <windows.h>

#include "type.h"

#include "core/platform/presentation_mailbox_interface.h"
#include "vm/platform/win32/w32adisp.h"

C_INT main(C_VOID)
{
    const COLORREF expected_background = RGB(0u, 0u, 255u);
    core_platform_presentation_mailbox *mailbox = STD_NULL;
    core_platform_display_frame frame = {0};
    w32adisp_context *context = STD_NULL;
    HWND window = STD_NULL;
    HDC dc = STD_NULL;
    C_INT failed = 0;

    window = CreateWindowEx(0u, "STATIC", "nxvm-text-palette-smoke", WS_POPUP,
        0, 0, 8, 16, STD_NULL, STD_NULL, GetModuleHandle(STD_NULL), STD_NULL);
    if (window == STD_NULL) return 1;
    ShowWindow(window, SW_SHOWNOACTIVATE);
    UpdateWindow(window);
    if (core_platform_presentation_mailbox_create(&mailbox) != TYPE_STATUS_OK) return 1;
    frame.kind = CORE_PLATFORM_DISPLAY_KIND_TEXT;
    frame.columns = 1u;
    frame.rows = 1u;
    frame.characters[0] = ' ';
    frame.attributes[0] = 0x14u;
    frame.text_glyphs_present = TYPE_TRUE;
    frame.generation = 1u;
    if (core_platform_presentation_mailbox_publish(mailbox, &frame) !=
        TYPE_STATUS_OK || (context = w32adisp_context_create()) == STD_NULL) {
        failed = 1;
        goto done;
    }
    w32adispInit(context, window, mailbox);
    dc = GetDC(window);
    if (dc == STD_NULL || GetPixel(dc, 0, 0) != expected_background) failed = 1;

done:
    if (dc != STD_NULL) ReleaseDC(window, dc);
    if (context != STD_NULL) {
        w32adispFinal(context);
        w32adisp_context_destroy(context);
    }
    core_platform_presentation_mailbox_destroy(mailbox);
    DestroyWindow(window);
    if (failed) return 1;
    STD_PRINTF("M5:T287:S18:W32-TEXT-PALETTE:OK\n");
    return 0;
}
