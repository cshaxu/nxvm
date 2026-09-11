#include "type.h"

#include "vm/product/machine_adapter.h"
#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/machine_private.h"
#include "../support/rom/session_assets.h"

typedef struct vm_console_lifecycle_adapter_fixture {
    C_INT reports;
    vm_machine_lifecycle lifecycle;
} vm_console_lifecycle_adapter_fixture;

static C_VOID vm_console_lifecycle_adapter_report(C_VOID *opaque,
    vm_machine_lifecycle lifecycle)
{
    vm_console_lifecycle_adapter_fixture *fixture =
        (vm_console_lifecycle_adapter_fixture *)opaque;

    if (fixture == STD_NULL) return;
    ++fixture->reports;
    fixture->lifecycle = lifecycle;
}

int main(C_VOID)
{
    vm_machine *session = STD_NULL;
    vm_product_machine_provider machine_provider;
    vm_console_lifecycle_adapter_fixture fixture = {0};
    C_INT passed = TYPE_FALSE;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK) goto done;
    vm_product_machine_provider_initialize(&machine_provider, &session);
    machine_provider.set_lifecycle_reporter(machine_provider.context,
        vm_console_lifecycle_adapter_report, &fixture);
    vm_machine_report_lifecycle(session, VM_MACHINE_PAUSED);
    passed = fixture.reports == 1 && fixture.lifecycle == VM_MACHINE_PAUSED;

done:
    vm_machine_destroy(session);
    if (!passed) return 1;
    STD_PRINTF("M5:T526:S3:CONSOLE-LIFECYCLE-ADAPTER:OK\n");
    return 0;
}
