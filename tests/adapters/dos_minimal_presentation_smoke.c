#include <stdio.h>

#include "vdm/platform/dos_minimal_presentation.h"

int main(void)
{
    nxvm_runtime_dos_minimal *session = NULL;
    nxvm_dos_minimal_presentation *presentation = NULL;
    nxvm_platform_input_event event = { 42u, 0x1eu };
    nxvm_platform_text_snapshot snapshot;
    uint32_t key;

    if (nxvm_runtime_dos_minimal_create(&session) != NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_create(session, &presentation) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_enqueue_input(presentation, event) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_apply_input(presentation) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_runtime_dos_minimal_port_read(session, 0x60u, &key) !=
            NXVM_CORE_STATUS_OK ||
        key != 0x1eu ||
        nxvm_runtime_dos_minimal_write_text(session, 0u, 'P', 0x2eu) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_dos_minimal_presentation_capture_text(presentation, 99u, &snapshot) !=
            NXVM_CORE_STATUS_OK ||
        snapshot.timestamp != 99u || snapshot.text.characters[0] != 'P') {
        nxvm_dos_minimal_presentation_destroy(presentation);
        nxvm_runtime_dos_minimal_destroy(session);
        return 1;
    }

    nxvm_dos_minimal_presentation_destroy(presentation);
    nxvm_runtime_dos_minimal_destroy(session);
    puts("M3:T4:S1:PRESENTATION:OK");
    return 0;
}
