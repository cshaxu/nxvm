#include <stdio.h>

#include "vdm/composition_presentation.h"

int main(void)
{
    nxvm_runtime_dos_minimal *first = NULL;
    nxvm_runtime_dos_minimal *second = NULL;
    nxvm_dos_minimal_presentation *first_presentation = NULL;
    nxvm_dos_minimal_presentation *second_presentation = NULL;
    nxvm_platform_input_event first_event = { 42u, 0x1eu };
    nxvm_platform_input_event second_event = { 43u, 0x30u };
    nxvm_vdm_presentation_snapshot first_snapshot;
    nxvm_vdm_presentation_snapshot second_snapshot;
    uint32_t key;

    if (nxvm_runtime_dos_minimal_create(&first) != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_dos_minimal_create(&second) != NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_create(first, &first_presentation) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_create(second, &second_presentation) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_enqueue_input(first_presentation,
                                                    first_event) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_enqueue_input(second_presentation,
                                                    second_event) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_apply_input(first_presentation) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_runtime_dos_minimal_port_read(first, 0x60u, &key) !=
            NXVM_CORE_STATUS_OK ||
        key != 0x1eu ||
        nxvm_runtime_dos_minimal_write_text(first, 0u, 'P', 0x2eu) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_capture_text(first_presentation, 99u,
                                                   &first_snapshot) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_capture_text(second_presentation, 100u,
                                                   &second_snapshot) !=
            NXVM_CORE_STATUS_OK ||
        first_snapshot.timestamp != 99u ||
        first_snapshot.text.characters[0] != 'P' ||
        second_snapshot.timestamp != 100u ||
        second_snapshot.text.characters[0] != ' ' ||
        nxvm_dos_minimal_presentation_apply_input(second_presentation) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_runtime_dos_minimal_port_read(second, 0x60u, &key) !=
            NXVM_CORE_STATUS_OK ||
        key != 0x30u) {
        nxvm_dos_minimal_presentation_destroy(second_presentation);
        nxvm_dos_minimal_presentation_destroy(first_presentation);
        nxvm_runtime_dos_minimal_destroy(second);
        nxvm_runtime_dos_minimal_destroy(first);
        return 1;
    }

    nxvm_dos_minimal_presentation_destroy(second_presentation);
    nxvm_dos_minimal_presentation_destroy(first_presentation);
    nxvm_runtime_dos_minimal_destroy(second);
    nxvm_runtime_dos_minimal_destroy(first);
    puts("M5:T94:S1:VDM-PRESENTATION-ISOLATION:OK");
    return 0;
}
