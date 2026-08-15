#include "type.h"

#include "core/platform/sleep.h"
#include "vm/composition/session/session.h"

C_INT main(C_VOID)
{
    const vm_session_config model_config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339
    };
    vm_session *model = STD_NULL;
    vm_session *default_session = STD_NULL;
    type_unsigned_64 ticks = 0u;
    C_INT failed = 0;

    failed |= vm_session_create(&model_config, &model) != TYPE_STATUS_OK ||
        model == STD_NULL || model->virtual_time_source.next == STD_NULL ||
        model->virtual_time_source.reset == STD_NULL;
    if (!failed) {
        failed |= model->virtual_time_source.next(
            model->virtual_time_source.context, &ticks) != TYPE_STATUS_OK || ticks != 0u;
        core_platform_sleep_milliseconds(2u);
        failed |= model->virtual_time_source.next(
            model->virtual_time_source.context, &ticks) != TYPE_STATUS_OK || ticks == 0u;
        model->virtual_time_source.reset(model->virtual_time_source.context);
        ticks = 1u;
        failed |= model->virtual_time_source.next(
            model->virtual_time_source.context, &ticks) != TYPE_STATUS_OK || ticks != 0u;
    }
    failed |= vm_session_create(STD_NULL, &default_session) != TYPE_STATUS_OK ||
        default_session == STD_NULL || default_session->virtual_time_source.next != STD_NULL;
    vm_session_destroy(default_session);
    vm_session_destroy(model);
    if (failed) return 1;
    STD_PRINTF("M5:T375:S17:MODEL339-VIRTUAL-TIME:OK\n");
    return 0;
}
