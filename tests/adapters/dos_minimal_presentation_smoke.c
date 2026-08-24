#include "type.h"



#include "vdm/composition/presentation.h"

C_INT main(C_VOID)
{
    vdm_machine_dos_minimal *first = STD_NULL;
    vdm_machine_dos_minimal *second = STD_NULL;
    vdm_presentation *first_presentation = STD_NULL;
    vdm_presentation *second_presentation = STD_NULL;
    vdm_composition_input_event first_event = { 42u, 0x5au };
    vdm_composition_input_event second_event = { 43u, 0x3au };
    vdm_presentation_snapshot first_snapshot;
    vdm_presentation_snapshot second_snapshot;
    type_unsigned_32 key;

    if (vdm_machine_dos_minimal_create(&first) != TYPE_STATUS_OK ||
        vdm_machine_dos_minimal_create(&second) != TYPE_STATUS_OK ||
        vdm_presentation_create(first, &first_presentation) !=
            TYPE_STATUS_OK ||
        vdm_presentation_create(second, &second_presentation) !=
            TYPE_STATUS_OK ||
        vdm_presentation_enqueue_input(first_presentation,
                                                    first_event) !=
            TYPE_STATUS_OK ||
        vdm_presentation_enqueue_input(second_presentation,
                                                    second_event) !=
            TYPE_STATUS_OK ||
        vdm_presentation_apply_input(first_presentation) !=
            TYPE_STATUS_OK ||
        vdm_machine_dos_minimal_port_read(first, 0x60u, &key) !=
            TYPE_STATUS_OK ||
        key != 0x1cu ||
        vdm_machine_dos_minimal_write_text(first, 0u, 'P', 0x2eu) !=
            TYPE_STATUS_OK ||
        vdm_presentation_capture_text(first_presentation, 99u,
                                                   &first_snapshot) !=
            TYPE_STATUS_OK ||
        vdm_presentation_capture_text(second_presentation, 100u,
                                                   &second_snapshot) !=
            TYPE_STATUS_OK ||
        first_snapshot.timestamp != 99u ||
        first_snapshot.text.characters[0] != 'P' ||
        second_snapshot.timestamp != 100u ||
        second_snapshot.text.characters[0] != ' ' ||
        vdm_presentation_apply_input(second_presentation) !=
            TYPE_STATUS_OK ||
        vdm_machine_dos_minimal_port_read(second, 0x60u, &key) !=
            TYPE_STATUS_OK ||
        key != 0x32u) {
        vdm_presentation_destroy(second_presentation);
        vdm_presentation_destroy(first_presentation);
        vdm_machine_dos_minimal_destroy(second);
        vdm_machine_dos_minimal_destroy(first);
        return 1;
    }

    vdm_presentation_destroy(second_presentation);
    vdm_presentation_destroy(first_presentation);
    vdm_machine_dos_minimal_destroy(second);
    vdm_machine_dos_minimal_destroy(first);
    puts("M5:T94:S1:VDM-PRESENTATION-ISOLATION:OK");
    return 0;
}
