#include "core/machine/display_interface.h"

#include <stddef.h>

static void *coreMachineDisplayContext;
static core_machine_display_provider coreMachineDisplayProvider;
static void *coreMachineDisplaySnapshotContext;
static core_machine_display_snapshot_provider coreMachineDisplaySnapshotProvider;

void core_machine_display_bind(void *context,
    core_machine_display_provider provider)
{
    coreMachineDisplayContext = context;
    coreMachineDisplayProvider = provider;
}

void core_machine_display_notify_mode_changed(void)
{
    if (coreMachineDisplayProvider != NULL) {
        coreMachineDisplayProvider(coreMachineDisplayContext);
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
