#include <stdio.h>
#include <stdlib.h>

#include "core/product/debug/debug_target.h"
#include "vm/composition_debug.h"
#include "vm/composition_machine.h"

int main(void)
{
    vm_composition_live_machine *session;
    const core_product_debug_target *target;
    uint32_t value = 0u;
    uint8_t byte = 0x5au;

    session = (vm_composition_live_machine *)calloc(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_initialize(session);
    target = vm_composition_debug_target(session);
    if (target == NULL || core_product_debug_scope_target() != NULL ||
        target->read_register(target->context,
            CORE_PRODUCT_DEBUG_EIP, &value) ||
        target->write_real(target->context, 0u, 0x500u, &byte, 1u) ||
        target->read_real(target->context, 0u, 0x500u, &value, 1u) ||
        (uint8_t)value != byte) {
        vm_composition_finalize(session);
        free(session);
        return 1;
    }
    vm_composition_finalize(session);
    free(session);
    if (core_product_debug_scope_target() != NULL) return 1;
    puts("M5:T14:S3:VM-DEBUG-TARGET:OK");
    return 0;
}
