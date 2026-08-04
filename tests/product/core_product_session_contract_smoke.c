#include "type.h"

#include "core/product/session/command_interface.h"
#include "core/product/session/session_provider.h"

C_INT main(C_VOID)
{
    core_product_session_snapshot snapshot = {
        .id = 0u,
        .selected = TYPE_TRUE,
        .state = CORE_PRODUCT_SESSION_STATE_READY,
        .display = CORE_PRODUCT_SESSION_DISPLAY_CONSOLE
    };
    core_product_session_provider provider = {0};
    core_product_session_output_provider output = {0};

    if (snapshot.id != 0u || !snapshot.selected ||
        snapshot.state != CORE_PRODUCT_SESSION_STATE_READY ||
        snapshot.display != CORE_PRODUCT_SESSION_DISPLAY_CONSOLE ||
        provider.context != STD_NULL || output.context != STD_NULL) return 1;
    STD_PRINTF("M5:T145:S1:SESSION-CONTRACT:OK\n");
    return 0;
}
