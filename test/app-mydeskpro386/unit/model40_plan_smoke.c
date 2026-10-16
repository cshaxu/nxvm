#include "app-mydeskpro386/machine_config.h"
#include "lib/types/types_interface.h"

lib_i32 main(void)
{
    x86_nxvm_board_contract_values values;

    return mydeskpro386_values_create(&values) != LIB_STATUS_OK ||
        values.core.configuration.memory_bytes != 2u * 1024u * 1024u ||
        values.core.configuration.cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        values.core.configuration.pic_topology != X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED ||
        values.core.configuration.dma_controller_count != X86_NXVM_BOARD_DMA_CONTROLLER_COUNT ||
        values.firmware_policy != X86_NXVM_BOARD_CONTRACT_FIRMWARE_POLICY_BYOB ||
        values.media_policy != X86_NXVM_BOARD_CONTRACT_MEDIA_POLICY_SESSION;
}
