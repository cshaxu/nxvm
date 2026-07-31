#include <stdio.h>

#include "products/nxvm/debugger.h"
#include "products/nxvm/presentation.h"

typedef struct input_fixture {
    uint16_t values[2];
    unsigned count;
} input_fixture;

static nxvm_core_status consume_input(void *context, uint16_t scan_code)
{
    input_fixture *fixture = (input_fixture *)context;
    if (fixture->count == 2u) return NXVM_CORE_STATUS_NO_MEMORY;
    fixture->values[fixture->count++] = scan_code;
    return NXVM_CORE_STATUS_OK;
}

int main(void)
{
    nxvm_product_nxvm_presentation presentation;
    nxvm_core_text_snapshot guest_text;
    nxvm_core_text_snapshot captured_text;
    input_fixture input = { { 0u, 0u }, 0u };
    nxvm_core_machine *machine = NULL;
    nxvm_core_machine_config config = {
        NXVM_CORE_ABI_VERSION, NXVM_CORE_PROFILE_TEST_MINIMAL, 0u
    };
    nxvm_product_nxvm_debugger debugger;
    nxvm_core_cpu_state cpu;
    nxvm_core_run_result result;

    nxvm_product_nxvm_presentation_initialize(&presentation);
    nxvm_core_text_snapshot_initialize(&guest_text);
    if (nxvm_product_nxvm_presentation_enqueue_input(&presentation, 0x1eu) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_presentation_enqueue_input(&presentation, 0x30u) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_presentation_apply_input(&presentation, consume_input,
            &input) != NXVM_CORE_STATUS_INVALID_STATE ||
        nxvm_product_nxvm_presentation_open_command_boundary(&presentation) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_presentation_apply_input(&presentation, consume_input,
            &input) != NXVM_CORE_STATUS_OK || input.count != 2u ||
        input.values[0] != 0x1eu || input.values[1] != 0x30u ||
        nxvm_core_text_snapshot_write(&guest_text, 0u, 0u, 'A', 7u) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_presentation_publish_text(&presentation,
            &guest_text) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_presentation_capture_text(&presentation,
            &captured_text) != NXVM_CORE_STATUS_OK || captured_text.characters[0] != 'A') return 1;
    nxvm_product_nxvm_presentation_close_command_boundary(&presentation);
    if (nxvm_product_nxvm_presentation_publish_text(&presentation,
            &guest_text) != NXVM_CORE_STATUS_INVALID_STATE ||
        nxvm_core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK ||
        nxvm_core_machine_reset(machine) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_debugger_initialize(&debugger, machine) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_debugger_read_cpu(&debugger, &cpu) != NXVM_CORE_STATUS_INVALID_STATE ||
        nxvm_product_nxvm_debugger_open_command_boundary(&debugger) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_debugger_read_cpu(&debugger, &cpu) != NXVM_CORE_STATUS_OK ||
        cpu.cs != 0xf000u ||
        nxvm_product_nxvm_debugger_step(&debugger, &result) != NXVM_CORE_STATUS_OK ||
        result.reason != NXVM_CORE_STOP_BUDGET) {
        nxvm_core_machine_destroy(machine);
        return 1;
    }
    nxvm_product_nxvm_debugger_close_command_boundary(&debugger);
    nxvm_core_machine_destroy(machine);
    puts("M5:T6:S2:NXVM-PRESENTATION-DEBUG:OK");
    return 0;
}
