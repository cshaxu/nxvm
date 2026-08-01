#include "core/machine/display.h"

#include <stddef.h>

static void *coreMachineDisplayContext;
static core_machine_display_mode_notifier coreMachineDisplayModeNotifier;

void core_machine_display_bind(void *context,
    core_machine_display_mode_notifier mode_notifier)
{
    coreMachineDisplayContext = context;
    coreMachineDisplayModeNotifier = mode_notifier;
}

void core_machine_display_notify_mode_changed(void)
{
    if (coreMachineDisplayModeNotifier != NULL) {
        coreMachineDisplayModeNotifier(coreMachineDisplayContext);
    }
}
