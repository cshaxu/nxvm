#ifndef VM_EVENTS_PRESENTATION_PLAN_H
#define VM_EVENTS_PRESENTATION_PLAN_H

#include "type.h"

#include "vm/events/machine_event.h"

typedef enum vm_presentation_surface {
    VM_PRESENTATION_SURFACE_NONE,
    VM_PRESENTATION_SURFACE_CONSOLE,
    VM_PRESENTATION_SURFACE_WINDOW
} vm_presentation_surface;

typedef enum vm_presentation_notice {
    VM_PRESENTATION_NOTICE_NONE,
    VM_PRESENTATION_NOTICE_STARTED,
    VM_PRESENTATION_NOTICE_RESUMED,
    VM_PRESENTATION_NOTICE_PAUSED,
    VM_PRESENTATION_NOTICE_RESET,
    VM_PRESENTATION_NOTICE_STOPPED
} vm_presentation_notice;

typedef struct vm_presentation_plan {
    C_INT target_changed;
    vm_presentation_surface target;
    C_INT title_changed;
    C_CHAR title[32];
    C_INT mouse_capturable_changed;
    C_INT mouse_capturable;
    C_INT release_mouse;
    C_INT frame_ready;
    vm_machine_display_event frame;
    vm_presentation_notice notice;
} vm_presentation_plan;

#endif
