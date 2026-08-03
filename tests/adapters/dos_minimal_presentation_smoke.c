#include "type.h"



#include "vdm/composition/composition_presentation.h"

C_INT main(C_VOID)
{
    core_product_runtime_dos_minimal *first = STD_NULL;
    core_product_runtime_dos_minimal *second = STD_NULL;
    vdm_composition_presentation *first_presentation = STD_NULL;
    vdm_composition_presentation *second_presentation = STD_NULL;
    nxvm_platform_input_event first_event = { 42u, 0x1eu };
    nxvm_platform_input_event second_event = { 43u, 0x30u };
    nxvm_vdm_presentation_snapshot first_snapshot;
    nxvm_vdm_presentation_snapshot second_snapshot;
    uint32_t key;

    if (core_product_runtime_dos_minimal_create(&first) != NTVDM64_STATUS_OK ||
        core_product_runtime_dos_minimal_create(&second) != NTVDM64_STATUS_OK ||
        vdm_composition_presentation_create(first, &first_presentation) !=
            NTVDM64_STATUS_OK ||
        vdm_composition_presentation_create(second, &second_presentation) !=
            NTVDM64_STATUS_OK ||
        vdm_composition_presentation_enqueue_input(first_presentation,
                                                    first_event) !=
            NTVDM64_STATUS_OK ||
        vdm_composition_presentation_enqueue_input(second_presentation,
                                                    second_event) !=
            NTVDM64_STATUS_OK ||
        vdm_composition_presentation_apply_input(first_presentation) !=
            NTVDM64_STATUS_OK ||
        core_product_runtime_dos_minimal_port_read(first, 0x60u, &key) !=
            NTVDM64_STATUS_OK ||
        key != 0x1eu ||
        core_product_runtime_dos_minimal_write_text(first, 0u, 'P', 0x2eu) !=
            NTVDM64_STATUS_OK ||
        vdm_composition_presentation_capture_text(first_presentation, 99u,
                                                   &first_snapshot) !=
            NTVDM64_STATUS_OK ||
        vdm_composition_presentation_capture_text(second_presentation, 100u,
                                                   &second_snapshot) !=
            NTVDM64_STATUS_OK ||
        first_snapshot.timestamp != 99u ||
        first_snapshot.text.characters[0] != 'P' ||
        second_snapshot.timestamp != 100u ||
        second_snapshot.text.characters[0] != ' ' ||
        vdm_composition_presentation_apply_input(second_presentation) !=
            NTVDM64_STATUS_OK ||
        core_product_runtime_dos_minimal_port_read(second, 0x60u, &key) !=
            NTVDM64_STATUS_OK ||
        key != 0x30u) {
        vdm_composition_presentation_destroy(second_presentation);
        vdm_composition_presentation_destroy(first_presentation);
        core_product_runtime_dos_minimal_destroy(second);
        core_product_runtime_dos_minimal_destroy(first);
        return 1;
    }

    vdm_composition_presentation_destroy(second_presentation);
    vdm_composition_presentation_destroy(first_presentation);
    core_product_runtime_dos_minimal_destroy(second);
    core_product_runtime_dos_minimal_destroy(first);
    puts("M5:T94:S1:VDM-PRESENTATION-ISOLATION:OK");
    return 0;
}
