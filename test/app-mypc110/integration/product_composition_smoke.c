#include "app-mypc110/product_binding_interface.h"
#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/pc110_audio.h"
#include "app-mypc110/firmware_variants.h"
#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/storage/file_interface.h"
#include "lib/base/sync_interface.h"
#include "x86-debug/product/protocol_interface.h"
#include "x86-nxvm/product/factory_interface.h"
#include "x86-nxvm/product/ini_interface.h"

#if defined(MYPC110_TEST_AUDIO_WRAP)
/* Observe the real Lib calls without changing its production interface. */
static lib_atomic_i32 audio_endpoint;
static lib_atomic_i32 audio_first_sample;
static lib_atomic_i32 audio_second_sample;

lib_status __real_lib_audio_pcm_stream_create(const lib_audio_pcm_stream_options *options,
    lib_audio_pcm_stream **out_stream);
lib_status __real_lib_audio_pcm_stream_enqueue(lib_audio_pcm_stream *stream,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted);

lib_status __wrap_lib_audio_pcm_stream_create(const lib_audio_pcm_stream_options *options,
    lib_audio_pcm_stream **out_stream)
{
    lib_status status = __real_lib_audio_pcm_stream_create(options, out_stream);
    lib_atomic_i32_store_explicit(&audio_endpoint,
        status == LIB_STATUS_OK && *out_stream != LIB_NULL, LIB_MEMORY_ORDER_SEQ_CST);
    return status;
}

lib_status __wrap_lib_audio_pcm_stream_enqueue(lib_audio_pcm_stream *stream,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted)
{
    lib_status status = __real_lib_audio_pcm_stream_enqueue(stream, samples, frame_count,
        out_accepted);
    for (lib_u32 i = 0u; i < *out_accepted; ++i) {
        if (samples[i] == -28160)
            lib_atomic_i32_store_explicit(&audio_first_sample, 1, LIB_MEMORY_ORDER_SEQ_CST);
        if (samples[i] == 25600)
            lib_atomic_i32_store_explicit(&audio_second_sample, 1, LIB_MEMORY_ORDER_SEQ_CST);
    }
    return status;
}
#endif

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

static void audio_paused(void *context, emulator_machine_state state, lib_u32 generation)
{
    (void)generation;
    if (state == EMULATOR_MACHINE_PAUSED || state == EMULATOR_MACHINE_ERROR)
        (void)base_sync_event_signal(context);
}

static void audio_guest_transfer(emulator_machine *machine,
    emulator_machine_debug_lease *lease, base_sync_event *event, lib_bool halt)
{
    static const lib_u16 ports[][2] = {
        {0x20u, 0x11u}, {0x21u, 0x08u}, {0x21u, 0x04u}, {0x21u, 0x01u},
        {0x0cu, 0u}, {0x02u, 0x00u}, {0x02u, 0x10u},
        {0x03u, 0x01u}, {0x03u, 0x00u}, {0x83u, 1u}, {0x0bu, 0x49u},
        {0x0au, 0x01u}, {0xd6u, 0xc0u}, {0xd4u, 0u},
        {0x22cu, 0xd1u}, {0x22cu, 0x40u}, {0x22cu, 156u},
        {0x22cu, 0x14u}, {0x22cu, 1u}, {0x22cu, 0u}};
    x86_debug_request request = {.operation = X86_DEBUG_WRITE_LINEAR,
        .address = 0x11000u, .bytes = 2u, .data = {0x12u, 0xe4u}};
    x86_debug_response response = {0};
    card_debug(machine, lease, &request, &response);
    request = (x86_debug_request){.operation = X86_DEBUG_WRITE_LINEAR,
        .address = 0x2000u, .bytes = 2u, .data = {0xebu, 0xfeu}};
    if (halt) { request.data[0] = 0xf4u; request.data[1] = 0x90u; }
    card_debug(machine, lease, &request, &response);
    if (halt) {
        request = (x86_debug_request){.operation = X86_DEBUG_WRITE_LINEAR,
            .address = 0x34u, .bytes = 4u, .data = {0x00u, 0x21u, 0x00u, 0x00u}};
        card_debug(machine, lease, &request, &response);
    }
    for (lib_size i = 0u; i < sizeof(ports) / sizeof(ports[0]); ++i) {
        request = (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
            .port = ports[i][0], .address = ports[i][1], .bytes = 1u};
        card_debug(machine, lease, &request, &response);
    }
    request = (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
        .port = 0x21u, .address = halt ? 0xdfu : 0xffu, .bytes = 1u};
    card_debug(machine, lease, &request, &response);
    request = (x86_debug_request){.operation = X86_DEBUG_WRITE_REGISTER,
        .register_id = X86_DEBUG_CS, .address = 0u};
    card_debug(machine, lease, &request, &response);
    request.register_id = X86_DEBUG_EIP;
    request.address = 0x2000u;
    card_debug(machine, lease, &request, &response);
    request.register_id = X86_DEBUG_EFLAGS;
    request.address = halt ? 0x202u : 2u;
    card_debug(machine, lease, &request, &response);
    if (halt) {
        request.register_id = X86_DEBUG_SS;
        request.address = 0u;
        card_debug(machine, lease, &request, &response);
        request.register_id = X86_DEBUG_ESP;
        request.address = 0x9000u;
        card_debug(machine, lease, &request, &response);
    }
    if (!halt) {
        request = (x86_debug_request){.operation = X86_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = X86_DEBUG_EXECUTION_TRACE, .instruction_count = 5000u};
        card_debug(machine, lease, &request, &response);
    } else {
        request = (x86_debug_request){.operation = X86_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = X86_DEBUG_EXECUTION_BREAK_LINEAR, .address = 0x2100u};
        card_debug(machine, lease, &request, &response);
    }
    emulator_machine_set_state_sink(machine, audio_paused, event);
    lib_test_assert(base_sync_event_reset(event) == LIB_STATUS_OK);
#if defined(MYPC110_TEST_AUDIO_WRAP)
    lib_atomic_i32_store_explicit(&audio_first_sample, 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&audio_second_sample, 0, LIB_MEMORY_ORDER_SEQ_CST);
#endif
    lib_test_assert(emulator_machine_resume(machine));
    lib_test_assert(base_sync_event_wait(event, 30000u) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(emulator_machine_state_get(machine) == EMULATOR_MACHINE_PAUSED);
#if defined(MYPC110_TEST_AUDIO_WRAP)
    if (lib_atomic_i32_load_explicit(&audio_endpoint, LIB_MEMORY_ORDER_SEQ_CST)) {
        lib_test_assert(lib_atomic_i32_load_explicit(&audio_first_sample, LIB_MEMORY_ORDER_SEQ_CST));
        lib_test_assert(lib_atomic_i32_load_explicit(&audio_second_sample, LIB_MEMORY_ORDER_SEQ_CST));
        lib_c_printf("PC110:GUEST-DMA-PCM:OK\n");
    } else lib_c_printf("PC110:GUEST-DMA-PCM:HOST-UNAVAILABLE\n");
#endif
    lib_test_assert(emulator_machine_debug_acquire(machine, lease) == LIB_STATUS_OK);
    if (halt) {
        request = (x86_debug_request){.operation = X86_DEBUG_READ_REGISTER,
            .register_id = X86_DEBUG_EIP};
        card_debug(machine, lease, &request, &response);
        lib_test_assert(response.value == 0x2100u);
    }
    request = (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
        .port = 0x20u, .address = halt ? 0x0bu : 0x0au, .bytes = 1u};
    card_debug(machine, lease, &request, &response);
    request = (x86_debug_request){.operation = X86_DEBUG_READ_PORT, .port = 0x20u, .bytes = 1u};
    card_debug(machine, lease, &request, &response);
    lib_test_assert((response.value & 0x20u) != 0u);
    request.port = 0x22eu;
    card_debug(machine, lease, &request, &response);
    if (halt) {
        request = (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
            .port = 0x20u, .address = 0x20u, .bytes = 1u};
        card_debug(machine, lease, &request, &response);
    }
    emulator_machine_set_state_sink(machine, card_state, event);
}

static void audio_native_delivery(void)
{
    mypc110_es488 device = {0};
    mypc110_audio audio;
    lib_audio_pcm_progress progress = {0};
    device.samples[0] = 0x40u;
    device.sample_periods[0] = 3300u;
    device.sample_count = 1u;
    lib_test_assert(mypc110_audio_open(&audio, &device) == LIB_STATUS_OK);
    if (audio.stream != LIB_NULL) {
        lib_test_assert(mypc110_audio_pump(&audio) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_flush(audio.stream) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_progress(audio.stream, &progress) == LIB_STATUS_OK);
        lib_test_assert(progress.accepted_frames == 4u && progress.submitted_frames == 4u);
        lib_c_printf("PC110:AUDIO-NATIVE-SUBMISSION:OK\n");
    } else {
        lib_c_printf("PC110:AUDIO-NATIVE-SUBMISSION:UNAVAILABLE\n");
    }
    lib_test_assert(mypc110_audio_close(&audio) == LIB_STATUS_OK);
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

    audio_native_delivery();
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
        audio_guest_transfer(executor, &lease, paused, LIB_FALSE);
        audio_guest_transfer(executor, &lease, paused, LIB_TRUE);
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
