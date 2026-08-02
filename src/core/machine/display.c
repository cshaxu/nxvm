#include "core/machine/display.h"

#include <stddef.h>

static void *coreMachineDisplayContext;
static core_machine_display_mode_notifier coreMachineDisplayModeNotifier;
static void *coreMachineDisplaySnapshotContext;
static core_machine_display_snapshot_provider coreMachineDisplaySnapshotProvider;

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

void core_machine_display_bind_snapshot_provider(void *context,
    core_machine_display_snapshot_provider provider)
{
    coreMachineDisplaySnapshotContext = context;
    coreMachineDisplaySnapshotProvider = provider;
}

int core_machine_display_capture_snapshot(core_machine_display_snapshot *out_snapshot)
{
    if (coreMachineDisplaySnapshotProvider == NULL || out_snapshot == NULL) {
        return 0;
    }
    return coreMachineDisplaySnapshotProvider(coreMachineDisplaySnapshotContext,
        out_snapshot);
}
