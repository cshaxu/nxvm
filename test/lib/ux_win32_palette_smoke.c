#include "lib/ux/frame.h"
#include "lib/ux/win32/geometry.h"

#include <stdio.h>

int main(void)
{
    static const uint32_t rgb[] = { 0x000000ffu, 0x00ff0000u, 0x00ffff00u };
    static const COLORREF colorref[] = {
        RGB(0u, 0u, 0xffu), RGB(0xffu, 0u, 0u), RGB(0xffu, 0xffu, 0u)
    };
    ux_frame frame = { 0 };
    size_t index;

    frame.valid = 1u;
    frame.text_columns = 80u;
    frame.text_rows = 25u;
    for (index = 0u; index < sizeof(rgb) / sizeof(rgb[0]); ++index) {
        frame.text_palette[index] = rgb[index];
        frame.graphics_palette[index] = rgb[index];
        if (frame.text_palette[index] != rgb[index] ||
            frame.graphics_palette[index] != rgb[index] ||
            ux_win32_colorref_from_rgb(frame.text_palette[index]) != colorref[index]) {
            return 1;
        }
    }
    if (!ux_frame_is_valid(&frame)) return 1;
    puts("M5:T524:S16:UX-WIN32-PALETTE:OK");
    return 0;
}
