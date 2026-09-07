#include "type.h"

#include "core/machine/guest_display_frame.h"
#include "core/machine/guest_presentation_mailbox_interface.h"
#include "lib/ux/internal/presenter_internal.h"
#include "lib/ux/presenter.h"
#include "vm/platform/platform.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/ux_frame.h"

C_INT main(C_VOID)
{
    static core_machine_guest_display_frame source;
    static ux_frame destination;
    static ux_frame captured;
    core_machine_guest_presentation_mailbox *core_mailbox = STD_NULL;
    vm_platform_run_context *context = STD_NULL;
    vm_platform_run_handle *handle = STD_NULL;
    ux_binding binding;
    type_unsigned_32 first_sequence;

    source.kind = CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT;
    source.generation = 7u;
    source.columns = 80u;
    source.rows = 25u;
    source.characters[0] = 'A';
    source.attributes[0] = 0x1eu;
    source.palette_rgb[14u] = 0x00ffff00u;
    if (vm_platform_ux_frame_from_core(&source, &destination) !=
            TYPE_STATUS_OK || !destination.valid || destination.graphics ||
        destination.text[0] != 'A' || destination.text_palette[14u] !=
            0x00ffff00u) return 1;
    if (core_machine_guest_presentation_mailbox_create(&core_mailbox) !=
            TYPE_STATUS_OK || vm_platform_run_context_create(STD_NULL, STD_NULL,
            core_mailbox, STD_NULL, &context) != TYPE_STATUS_OK ||
        vm_platform_run_handle_create(&handle) != TYPE_STATUS_OK ||
        vm_platform_ux_binding_initialize(context, handle, &binding) !=
            TYPE_STATUS_OK || core_machine_guest_presentation_mailbox_publish(
            core_mailbox, &source) != TYPE_STATUS_OK ||
        vm_platform_run_context_publish_ux_frame(context) != TYPE_STATUS_OK ||
        ux_presenter_capture_frame(binding.presenter, &captured) != LIB_STATUS_OK ||
        captured.text[0] != 'A') goto fail;
    first_sequence = captured.sequence;
    source.generation = 8u;
    source.characters[0] = 'B';
    if (core_machine_guest_presentation_mailbox_publish(core_mailbox, &source) !=
            TYPE_STATUS_OK || vm_platform_run_context_publish_ux_frame(context) !=
            TYPE_STATUS_OK || ux_presenter_capture_frame(binding.presenter,
            &captured) != LIB_STATUS_OK || captured.sequence <= first_sequence ||
        captured.text[0] != 'B') goto fail;
    source.kind = CORE_MACHINE_GUEST_DISPLAY_KIND_INDEXED_PIXELS;
    source.generation = 9u;
    source.pixel_width = 320u;
    source.pixel_height = 200u;
    if (core_machine_guest_presentation_mailbox_publish(core_mailbox, &source) !=
            TYPE_STATUS_OK || vm_platform_run_context_publish_ux_frame(context) !=
            TYPE_STATUS_OK || context->requested_target != UX_TARGET_WINDOW)
        goto fail;
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    core_machine_guest_presentation_mailbox_destroy(core_mailbox);
    return 0;

fail:
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    core_machine_guest_presentation_mailbox_destroy(core_mailbox);
    return 1;
}
