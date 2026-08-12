#include "type.h"

#include "vm/profile/default_profile/mouse_mapper.h"

type_status vm_profile_default_mouse_map_host_relative(type_signed_16 host_delta_x,
    type_signed_16 host_delta_y, type_unsigned_8 host_buttons,
    vm_profile_default_mouse_report *out_report)
{
    if (out_report == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    out_report->delta_x = host_delta_x;
    /* Win32 and the retained host coordinate convention grow downward; PS/2
     * relative Y grows upward. INT16_MIN is clamped deterministically. */
    out_report->delta_y = host_delta_y == INT16_MIN ? INT16_MAX :
        (type_signed_16)-host_delta_y;
    out_report->buttons = host_buttons & 0x07u;
    return TYPE_STATUS_OK;
}
