#include <stdio.h>

#include "vm/composition_debug.h"
#include "vm/composition_machine.h"

int main(void)
{
    const core_product_debug_target *target;
    uint32_t value = 0u;
    uint8_t byte = 0x5au;

    machineInit();
    target = vm_composition_debug_target();
    if (target == NULL || target->read_register(target->context,
            CORE_PRODUCT_DEBUG_EIP, &value) ||
        target->write_real(target->context, 0u, 0x500u, &byte, 1u) ||
        target->read_real(target->context, 0u, 0x500u, &value, 1u) ||
        (uint8_t)value != byte) {
        machineFinal();
        return 1;
    }
    machineFinal();
    puts("M5:T14:S3:VM-DEBUG-TARGET:OK");
    return 0;
}
