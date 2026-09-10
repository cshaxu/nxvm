#include "type.h"

#include "core/product/session/session_interface.h"
#include "vm/composition/session/console_machine_adapter.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/provider.h"
#include "vm/composition/session/session_private.h"

typedef struct vm_console_lifecycle_adapter_fixture {
    C_INT reports;
    core_product_session_id id;
    vm_session_lifecycle lifecycle;
} vm_console_lifecycle_adapter_fixture;

static C_VOID vm_console_lifecycle_adapter_report(C_VOID *opaque,
    core_product_session_id id, vm_session_lifecycle lifecycle)
{
    vm_console_lifecycle_adapter_fixture *fixture =
        (vm_console_lifecycle_adapter_fixture *)opaque;

    if (fixture == STD_NULL) return;
    ++fixture->reports;
    fixture->id = id;
    fixture->lifecycle = lifecycle;
}

static type_status vm_console_lifecycle_adapter_emit(C_VOID *opaque,
    C_VOID *session)
{
    (C_VOID)opaque;
    vm_session_report_lifecycle((vm_session *)session, VM_SESSION_PAUSED);
    return TYPE_STATUS_OK;
}

int main(C_VOID)
{
    core_product_session_provider session_provider;
    core_product_session_manager *manager = STD_NULL;
    vm_session_machine_provider machine_provider;
    vm_console_lifecycle_adapter_fixture fixture = {0};
    C_INT passed = TYPE_FALSE;

    vm_session_provider_initialize(&session_provider);
    if (core_product_session_manager_create(&session_provider, &manager) !=
            TYPE_STATUS_OK) goto done;
    vm_composition_console_machine_provider_initialize(&machine_provider, manager);
    machine_provider.set_lifecycle_reporter(machine_provider.context,
        vm_console_lifecycle_adapter_report, &fixture);
    if (core_product_session_manager_apply_all(manager,
            vm_console_lifecycle_adapter_emit, STD_NULL) != TYPE_STATUS_OK) goto done;
    passed = fixture.reports == 1 && fixture.id == 1u &&
        fixture.lifecycle == VM_SESSION_PAUSED;

done:
    core_product_session_manager_destroy(manager);
    if (!passed) return 1;
    STD_PRINTF("M5:T526:S3:CONSOLE-LIFECYCLE-ADAPTER:OK\n");
    return 0;
}
