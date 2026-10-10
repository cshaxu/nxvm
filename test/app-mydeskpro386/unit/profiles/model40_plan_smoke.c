#include "app-mydeskpro386/profiles/model40_private.h"
#include "lib/types/types_interface.h"

lib_i32 main(void)
{
    vm_profile_contract_values values;

    return vm_profile_model40_values_create(&values) != LIB_STATUS_OK ||
        values.core.configuration.memory_bytes != 2u * 1024u * 1024u ||
        values.core.configuration.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        values.core.configuration.pic_topology != CORE_MACHINE_PIC_TOPOLOGY_CASCADED ||
        values.core.configuration.dma_controller_count != CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        values.firmware_policy != VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BYOB ||
        values.media_policy != VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION;
}
