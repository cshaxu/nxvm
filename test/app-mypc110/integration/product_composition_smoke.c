#include "app-mypc110/product_binding_interface.h"
#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/firmware_variants.h"
#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/storage/file_interface.h"
#include "lib/base/sync_interface.h"
#include "x86-debug/product/protocol_interface.h"
#include "x86-nxvm/product/factory_interface.h"
#include "x86-nxvm/product/ini_interface.h"

static void card_state(void *context, emulator_machine_state state, lib_u32 generation)
{
    (void)generation;
    if (state == EMULATOR_MACHINE_RESET_COMPLETED || state == EMULATOR_MACHINE_STOPPED ||
        state == EMULATOR_MACHINE_ERROR)
        (void)base_sync_event_signal(context);
}

static void card_debug(emulator_machine *machine, const emulator_machine_debug_lease *lease,
    x86_debug_request *request, x86_debug_response *response)
{
    lib_size bytes = 0u;
    lib_test_assert(emulator_machine_debug_execute_with_lease(machine, lease, request,
        sizeof(*request), response, sizeof(*response), &bytes) == LIB_STATUS_OK);
    lib_test_assert(bytes == sizeof(*response));
}

static void card_register(emulator_machine *machine, const emulator_machine_debug_lease *lease,
    lib_u8 index, lib_u8 value)
{
    x86_debug_response response = {0};
    x86_debug_request request = {.operation = X86_DEBUG_WRITE_PORT, .port = 0x03e0u,
        .address = index, .bytes = 1u};
    card_debug(machine, lease, &request, &response);
    request.port = 0x03e1u;
    request.address = value;
    card_debug(machine, lease, &request, &response);
}

int main(void)
{
    lib_u8 document[] = "[machine]\n"
        "memory_kib=20480\n"
        "[presentation]\n"
        "display=console\n"
        "console_control=0\n"
        "[media]\nfixed_disk0=pc110-vdisplay-direct.img|direct\n";
    x86_nxvm_product_request request = {0};
    x86_nxvm_product_machine machine = {0};
    x86_nxvm_product_information information = {0};
    x86_nxvm_product_machine_binding binding = vm_app_machine;
    mypc110_product_context product = {0};
    mypc110_vdisplay *display = &product.display;
    static const lib_u8 zeroes[4096u];
    lib_storage_file_writer *writer = LIB_NULL;
    lib_storage_medium *second = LIB_NULL;
    int failed = 0;

    lib_test_assert(lib_storage_file_writer_open("pc110-vdisplay-direct.img",
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
    for (lib_size i = 0u; i < 984u; ++i)
        lib_test_assert(lib_storage_file_writer_write(writer, zeroes, sizeof(zeroes)) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);

    mypc110_vdisplay_initialize(display);
    mypc110_product_cards_initialize(&product.cards);
    display->firmware[0] = &mypc110_dstn_firmware;
    display->firmware[1] = &mypc110_tft_firmware;
    binding.context = &product;

    /* This integration owns no executable-adjacent user state. The process
     * Product integration tests exercise the actual file-backed binding. */
    binding.prepare = mypc110_machine_construction_create;
    binding.save_configuration = LIB_NULL;
    if (x86_nxvm_product_ini_parse((const lib_u8 *)".",
            (const lib_u8 *)"mypc110.ini", document, &request) != LIB_STATUS_OK ||
        request.memory_bytes != 20u * 1024u * 1024u ||
        x86_nxvm_product_compose_machine(&binding, &request, &machine) !=
            LIB_STATUS_OK) {
        return 1;
    }
    if (machine.composition.machine == LIB_NULL ||
        machine.composition.driver.copy_frame == LIB_NULL ||
        machine.information(machine.context, machine.composition.machine,
            &information) != LIB_STATUS_OK ||
        lib_text_compare(information.machine_name, "ibm-pc110") != 0 ||
        information.memory_bytes != 20u * 1024u * 1024u ||
        vm_app_machine.firmware == LIB_NULL || vm_app_machine.firmware->bios[0].data == LIB_NULL ||
        vm_app_machine.firmware->bios[0].bytes != 256u * 1024u ||
        vm_app_machine.firmware->bios[0].data[0x20af6u] != 0x15u) {
        failed = 2;
    }
    for (lib_size index = 0u; index < 4u; ++index) {
        const mypc110_display_variant selected = (index & 1u) == 0u ?
            MYPC110_DISPLAY_TFT : MYPC110_DISPLAY_DSTN;
        void *identity = machine.composition.machine;
        lib_test_assert(lib_storage_medium_open("pc110-vdisplay-direct.img",
            LIB_STORAGE_MEDIUM_DIRECT, &second) != LIB_STATUS_OK && second == LIB_NULL);
        lib_atomic_i32_store_explicit(&display->requested, selected, LIB_MEMORY_ORDER_SEQ_CST);
        lib_test_assert(machine.composition.driver.reset(machine.composition.driver.context));
        lib_test_assert(display->active == selected && machine.composition.machine == identity);
        lib_test_assert(display->present[1u - selected] && display->dirty[1u - selected]);
    }
    {
        emulator_machine *executor = LIB_NULL;
        emulator_machine_debug_lease lease = {0};
        emulator_session_command_result result = {0};
        x86_ibmpc_product_command_extensions extensions = {0};
        base_sync_event *paused = LIB_NULL;
        x86_debug_request read = {.operation = X86_DEBUG_READ_LINEAR,
            .address = 0xc0000u, .bytes = 1u};
        x86_debug_response response = {0};
        const lib_u8 card_data[] = {0x6du};

        lib_test_assert(lib_storage_file_writer_open("pc110-card-common.img",
            LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_write(writer, card_data, sizeof(card_data)) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_write(writer, zeroes, sizeof(zeroes)) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_open("pc110-card-direct.img",
            LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_write(writer, zeroes, sizeof(zeroes)) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_open("pc110-card-attribute.img",
            LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_write(writer, zeroes, sizeof(zeroes)) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &paused) == LIB_STATUS_OK);
        lib_test_assert(emulator_machine_create(&executor, &machine.composition.driver) == LIB_STATUS_OK);
        lib_test_assert(machine.composition.bind(machine.composition.machine, executor) == LIB_STATUS_OK);
        lib_test_assert(binding.configure_extensions(binding.context, &machine, &extensions) == LIB_STATUS_OK);
        emulator_machine_set_state_sink(executor, card_state, paused);
        lib_test_assert(extensions.submit(extensions.context, executor,
            EMULATOR_SESSION_MACHINE_STOPPED,
            "pccard insert a overlay pc110-card-common.img|pc110-card-attribute.img", &result));
        lib_test_assert(product.cards.slot[0].pending_common != LIB_NULL);
        lib_test_assert(emulator_machine_reset(executor));
        lib_test_assert(base_sync_event_wait(paused, 30000u) == BASE_SYNC_WAIT_SIGNALED);
        lib_test_assert(emulator_machine_state_get(executor) == EMULATOR_MACHINE_PAUSED);
        lib_test_assert(product.cards.slot[0].active_common != LIB_NULL);
        lib_test_assert(emulator_machine_debug_acquire(executor, &lease) == LIB_STATUS_OK);
        card_register(executor, &lease, 0x10u, 0xc0u);
        card_register(executor, &lease, 0x11u, 0x80u);
        card_register(executor, &lease, 0x12u, 0xc0u);
        card_register(executor, &lease, 0x13u, 0u);
        card_register(executor, &lease, 0x14u, 0x40u);
        card_register(executor, &lease, 0x15u, 0x3fu);
        card_register(executor, &lease, 0x06u, 1u);
        card_debug(executor, &lease, &read, &response);
        lib_test_assert(response.data[0] == card_data[0]);
        card_register(executor, &lease, 0x15u, 0x7fu);
        {
            x86_debug_request write = {.operation = X86_DEBUG_WRITE_LINEAR,
                .address = 0xc0000u, .bytes = 1u, .data = {0x35u}};
            card_debug(executor, &lease, &write, &response);
            card_debug(executor, &lease, &read, &response);
            lib_test_assert(response.data[0] == 0x35u);
        }
        lib_test_assert(extensions.submit(extensions.context, executor,
            EMULATOR_SESSION_MACHINE_PAUSED, "pccard eject a", &result));
        lib_test_assert(product.cards.slot[0].pending_common == LIB_NULL);
        lib_test_assert(emulator_machine_stop(executor));
        lib_test_assert(base_sync_event_wait(paused, 30000u) == BASE_SYNC_WAIT_SIGNALED);
        lib_test_assert(emulator_machine_state_get(executor) == EMULATOR_MACHINE_STOPPED);
        lib_test_assert(extensions.submit(extensions.context, executor,
            EMULATOR_SESSION_MACHINE_STOPPED,
            "pccard insert b readonly missing-card.img|pc110-card-attribute.img", &result));
        lib_test_assert(product.cards.slot[1].pending_common == LIB_NULL);
        lib_test_assert(product.cards.slot[0].active_common != LIB_NULL);
        lib_test_assert(extensions.submit(extensions.context, executor,
            EMULATOR_SESSION_MACHINE_STOPPED,
            "pccard insert b direct pc110-card-direct.img|pc110-card-attribute.img", &result));
        lib_test_assert(product.cards.slot[1].pending_common != LIB_NULL);
        {
            lib_storage_medium *pending = product.cards.slot[1].pending_common;
            lib_test_assert(extensions.submit(extensions.context, executor,
                EMULATOR_SESSION_MACHINE_STOPPED,
                "pccard insert b overlay pc110-card-common.img|missing-attribute.img", &result));
            lib_test_assert(product.cards.slot[1].pending_common == pending);
        }
        lib_test_assert(extensions.submit(extensions.context, executor,
            EMULATOR_SESSION_MACHINE_STOPPED,
            "pccard insert b readonly pc110-card-common.img|pc110-card-attribute.img", &result));
        lib_test_assert(lib_storage_medium_open("pc110-card-direct.img",
            LIB_STORAGE_MEDIUM_DIRECT, &second) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_medium_destroy(&second) == LIB_STATUS_OK);
        lib_test_assert(extensions.submit(extensions.context, executor,
            EMULATOR_SESSION_MACHINE_STOPPED, "pccard eject a", &result));
        lib_test_assert(emulator_machine_reset(executor));
        lib_test_assert(base_sync_event_wait(paused, 30000u) == BASE_SYNC_WAIT_SIGNALED);
        lib_test_assert(emulator_machine_state_get(executor) == EMULATOR_MACHINE_PAUSED);
        lib_test_assert(product.cards.slot[0].active_common == LIB_NULL);
        lib_test_assert(product.cards.slot[1].active_common != LIB_NULL);
        lib_test_assert(emulator_machine_debug_acquire(executor, &lease) == LIB_STATUS_OK);
        card_register(executor, &lease, 0x50u, 0xc0u);
        card_register(executor, &lease, 0x51u, 0u);
        card_register(executor, &lease, 0x52u, 0xc0u);
        card_register(executor, &lease, 0x53u, 0u);
        card_register(executor, &lease, 0x54u, 0x40u);
        card_register(executor, &lease, 0x55u, 0x3fu);
        card_register(executor, &lease, 0x46u, 1u);
        card_debug(executor, &lease, &read, &response);
        lib_test_assert(response.data[0] == card_data[0]);
        lib_test_assert(emulator_machine_shutdown(executor) == LIB_STATUS_OK);
        lib_test_assert(machine.composition.bind(machine.composition.machine, LIB_NULL) == LIB_STATUS_OK);
        lib_test_assert(mypc110_machine_release_cards(&product) == LIB_STATUS_OK);
        lib_test_assert(emulator_machine_destroy(executor) == LIB_STATUS_OK);
        base_sync_event_destroy(paused);
        lib_test_assert(lib_storage_medium_open("pc110-card-attribute.img",
            LIB_STORAGE_MEDIUM_READONLY, &second) == LIB_STATUS_OK);
        {
            lib_u8 original = 0xffu;
            lib_test_assert(lib_storage_medium_read_at(second, 0u, &original, 1u) == LIB_STATUS_OK);
            lib_test_assert(original == 0u);
        }
        lib_test_assert(lib_storage_medium_destroy(&second) == LIB_STATUS_OK);
        lib_test_assert(lib_c_remove("pc110-card-common.img") == 0);
        lib_test_assert(lib_c_remove("pc110-card-attribute.img") == 0);
        lib_test_assert(lib_c_remove("pc110-card-direct.img") == 0);
    }
    lib_test_assert(mypc110_machine_release_cards(&product) == LIB_STATUS_OK);
    lib_test_assert(mypc110_product_cards_close_pending(&product.cards) == LIB_STATUS_OK);
    if (machine.composition.destroy(machine.composition.machine) != LIB_STATUS_OK)
        failed = 3;
    lib_test_assert(lib_storage_medium_open("pc110-vdisplay-direct.img",
        LIB_STORAGE_MEDIUM_DIRECT, &second) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_destroy(&second) == LIB_STATUS_OK);
    lib_test_assert(lib_c_remove("pc110-vdisplay-direct.img") == 0);
    return failed;
}
