#include "type.h"

#include "core/platform/display_frame.h"
#include "core/platform/presentation_mailbox_interface.h"
#include "lib/ux/presenter.h"
#include "vm/platform/platform.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/ux_frame.h"

C_INT main(C_VOID)
{
    static core_platform_display_frame source;
    static ux_frame destination;
    static ux_frame captured;
    core_platform_presentation_mailbox *core_mailbox = STD_NULL;
    vm_platform_run_context *context = STD_NULL;
    vm_platform_run_handle *handle = STD_NULL;
    ux_binding binding;

    source.kind = CORE_PLATFORM_DISPLAY_KIND_TEXT;
    source.generation = 7u;
    source.characters[0] = 'A';
    source.attributes[0] = 0x1eu;
    source.palette_rgb[0u] = 0x00000000u;
    source.palette_rgb[14u] = 0x00ffff00u;
    source.cursor_visible = TYPE_TRUE;
    source.cursor_x = 1u;
    source.cursor_y = 2u;
    source.cursor_top = 14u;
    source.cursor_bottom = 15u;
    if (vm_platform_ux_frame_from_core(&source, &destination) !=
            TYPE_STATUS_OK || !destination.valid || destination.graphics ||
        destination.sequence != 7u || destination.text[0] != 'A' ||
        destination.attributes[0] != 0x1eu ||
        destination.text_palette[14u] != 0x00ffff00u ||
        destination.cursor_column != 1 || destination.cursor_row != 2 ||
        destination.cursor_size != 2u) return 1;
    if (core_platform_presentation_mailbox_create(&core_mailbox) != TYPE_STATUS_OK ||
        vm_platform_run_context_create(STD_NULL, STD_NULL, core_mailbox,
            STD_NULL, &context) != TYPE_STATUS_OK ||
        vm_platform_run_handle_create(&handle) != TYPE_STATUS_OK ||
        vm_platform_ux_binding_initialize(context, handle, &binding) !=
            TYPE_STATUS_OK || core_platform_presentation_mailbox_publish(
            core_mailbox, &source) != TYPE_STATUS_OK ||
        vm_platform_run_context_publish_ux_frame(context) != TYPE_STATUS_OK ||
        ux_mailbox_capture(binding.mailbox, &captured) != TYPE_STATUS_OK ||
        captured.sequence == 0u || captured.text[0] != 'A' ||
        captured.text_palette[14u] != 0x00ffff00u) goto fail;
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    core_platform_presentation_mailbox_destroy(core_mailbox);
    puts("M5:T522:S4:UX-FRAME:OK");
    return 0;

fail:
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    core_platform_presentation_mailbox_destroy(core_mailbox);
    return 1;
}
