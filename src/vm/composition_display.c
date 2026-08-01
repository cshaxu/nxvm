#include "core/machine/display.h"
#include "vm/composition_display.h"
#include "vm/platform/platform.h"

static void vmCompositionDisplayModeChanged(void *context)
{
    (void)context;
    platformDisplaySetScreen();
}

void vm_composition_bind_display(void)
{
    core_machine_display_bind(NULL, vmCompositionDisplayModeChanged);
}
