#include "type.h"



#include "vdm/composition/presentation.h"

C_INT main(C_VOID)
{
    vdm_session *first = STD_NULL;
    vdm_session *second = STD_NULL;
    vdm_presentation *first_presentation = STD_NULL;
    vdm_presentation *second_presentation = STD_NULL;
    nxvm_platform_input_event first_event = { 42u, 0x1eu };
    nxvm_platform_input_event second_event = { 43u, 0x30u };
    vdm_presentation_snapshot first_snapshot;
    vdm_presentation_snapshot second_snapshot;
    uint32_t key;

    if (vdm_session_create(&first) != NTVDM64_STATUS_OK ||
        vdm_session_create(&second) != NTVDM64_STATUS_OK ||
        vdm_presentation_create(first, &first_presentation) !=
            NTVDM64_STATUS_OK ||
        vdm_presentation_create(second, &second_presentation) !=
            NTVDM64_STATUS_OK ||
        vdm_presentation_enqueue_input(first_presentation,
                                                    first_event) !=
            NTVDM64_STATUS_OK ||
        vdm_presentation_enqueue_input(second_presentation,
                                                    second_event) !=
            NTVDM64_STATUS_OK ||
        vdm_presentation_apply_input(first_presentation) !=
            NTVDM64_STATUS_OK ||
        vdm_session_port_read(first, 0x60u, &key) !=
            NTVDM64_STATUS_OK ||
        key != 0x1eu ||
        vdm_session_write_text(first, 0u, 'P', 0x2eu) !=
            NTVDM64_STATUS_OK ||
        vdm_presentation_capture_text(first_presentation, 99u,
                                                   &first_snapshot) !=
            NTVDM64_STATUS_OK ||
        vdm_presentation_capture_text(second_presentation, 100u,
                                                   &second_snapshot) !=
            NTVDM64_STATUS_OK ||
        first_snapshot.timestamp != 99u ||
        first_snapshot.text.characters[0] != 'P' ||
        second_snapshot.timestamp != 100u ||
        second_snapshot.text.characters[0] != ' ' ||
        vdm_presentation_apply_input(second_presentation) !=
            NTVDM64_STATUS_OK ||
        vdm_session_port_read(second, 0x60u, &key) !=
            NTVDM64_STATUS_OK ||
        key != 0x30u) {
        vdm_presentation_destroy(second_presentation);
        vdm_presentation_destroy(first_presentation);
        vdm_session_destroy(second);
        vdm_session_destroy(first);
        return 1;
    }

    vdm_presentation_destroy(second_presentation);
    vdm_presentation_destroy(first_presentation);
    vdm_session_destroy(second);
    vdm_session_destroy(first);
    puts("M5:T94:S1:VDM-PRESENTATION-ISOLATION:OK");
    return 0;
}

