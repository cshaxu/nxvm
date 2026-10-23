#include "lib/types/types_interface.h"
#include "lib/base/sync_interface.h"
#include "test/x86-nxvm/setup/session_ini.h"

typedef struct frame_probe {
    emulator_machine *machine;
    lib_atomic_i32 count;
    lib_atomic_i32 invalid;
    kvm_window_frame_storage frame;
} frame_probe;

static void observe_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation)
{
    frame_probe *probe = opaque;
    (void)sequence;
    (void)graphics;
    if (!emulator_machine_copy_published_frame(probe->machine, &probe->frame,
            generation)) {
        lib_atomic_i32_store_explicit(&probe->invalid, 1, LIB_MEMORY_ORDER_SEQ_CST);
        return;
    }
    if (probe->frame.frame.text != LIB_NULL &&
        probe->frame.data.text->base.text_columns == 1u &&
        probe->frame.data.text->base.text_rows == 1u &&
        probe->frame.data.text->base.font_height == 1u)
        lib_atomic_i32_store_explicit(&probe->invalid, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_fetch_add_explicit(&probe->count, 1, LIB_MEMORY_ORDER_SEQ_CST);
}

static lib_bool wait_for_frame(frame_probe *probe, lib_i32 previous)
{
    for (lib_u32 elapsed = 0u; elapsed < 60000u; elapsed += 10u) {
        if (lib_atomic_i32_load_explicit(&probe->invalid, LIB_MEMORY_ORDER_SEQ_CST))
            return LIB_FALSE;
        if (lib_atomic_i32_load_explicit(&probe->count, LIB_MEMORY_ORDER_SEQ_CST) > previous)
            return LIB_TRUE;
        base_sync_sleep_milliseconds(10u);
    }
    return LIB_FALSE;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session session;
    emulator_machine_driver driver;
    frame_probe probe = {0};
    lib_i32 failed = 1;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2], &session) != LIB_STATUS_OK)
        return 77;
    lib_atomic_i32_initialize(&probe.count, 0);
    lib_atomic_i32_initialize(&probe.invalid, 0);
    if (x86_nxvm_machine_describe_emulator_driver(session.session, &driver) != LIB_STATUS_OK ||
        emulator_machine_create(&session.emulator_machine, &driver) != LIB_STATUS_OK ||
        x86_nxvm_machine_bind_emulator_machine(session.session, session.emulator_machine) != LIB_STATUS_OK)
        goto done;
    probe.machine = session.emulator_machine;
    emulator_machine_set_frame_sink(probe.machine, observe_frame, &probe);
    if (!emulator_machine_start(probe.machine) || !wait_for_frame(&probe, 0)) goto done;
    if (integration_ini_session_pause(&session, 5000u) != LIB_STATUS_OK ||
        integration_ini_session_reset(&session, 5000u) != LIB_STATUS_OK ||
        lib_atomic_i32_load_explicit(&probe.invalid, LIB_MEMORY_ORDER_SEQ_CST)) goto done;
    {
        lib_i32 previous = lib_atomic_i32_load_explicit(&probe.count, LIB_MEMORY_ORDER_SEQ_CST);
        if (integration_ini_session_resume(&session, 5000u) != LIB_STATUS_OK ||
            !wait_for_frame(&probe, previous)) goto done;
    }
    if (!emulator_machine_stop(probe.machine) ||
        !integration_ini_session_wait_for_state(&session, EMULATOR_MACHINE_STOPPED, 5000u)) goto done;
    {
        lib_i32 previous = lib_atomic_i32_load_explicit(&probe.count, LIB_MEMORY_ORDER_SEQ_CST);
        if (!emulator_machine_start(probe.machine) || !wait_for_frame(&probe, previous)) goto done;
    }
    failed = !lib_atomic_i32_load_explicit(&probe.invalid, LIB_MEMORY_ORDER_SEQ_CST) ? 0 : 1;
done:
    integration_ini_session_close(&session);
    kvm_window_frame_release(&probe.frame);
    return failed;
}
