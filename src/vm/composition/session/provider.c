#include "type.h"

#include "vm/composition/session/provider.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/debug_target.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/providers.h"
#include "core/machine/memory.h"
#include "core/product/debug/debug.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"
#include "vm/profile/default_profile/firmware/bios.h"

static vm_session *vm_session_provider_selected(C_VOID *context)
{
    C_VOID *session = STD_NULL;

    if (core_product_session_manager_borrow_selected(
            (core_product_session_manager *)context, &session) != NTVDM64_STATUS_OK) {
        return STD_NULL;
    }
    return (vm_session *)session;
}

static C_INT vm_session_provider_parse_cpu(const C_CHAR *value,
    core_machine_cpu_profile *out_profile)
{
    if (!STD_STRCMP(value, "8086")) *out_profile = CORE_MACHINE_CPU_PROFILE_8086;
    else if (!STD_STRCMP(value, "80186")) *out_profile = CORE_MACHINE_CPU_PROFILE_80186;
    else if (!STD_STRCMP(value, "80286")) *out_profile = CORE_MACHINE_CPU_PROFILE_80286;
    else if (!STD_STRCMP(value, "80386")) *out_profile = CORE_MACHINE_CPU_PROFILE_80386;
    else return 0;
    return 1;
}

static C_INT vm_session_provider_parse_fpu(const C_CHAR *value,
    core_machine_fpu_profile *out_profile)
{
    if (!STD_STRCMP(value, "none")) *out_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    else if (!STD_STRCMP(value, "8087")) *out_profile = CORE_MACHINE_FPU_PROFILE_8087;
    else if (!STD_STRCMP(value, "80287")) *out_profile = CORE_MACHINE_FPU_PROFILE_80287;
    else if (!STD_STRCMP(value, "80387")) *out_profile = CORE_MACHINE_FPU_PROFILE_80387;
    else return 0;
    return 1;
}

static ntvdm64_status vm_session_provider_parse_options(
    const core_product_session_open_options *options, vm_session_config *config)
{
    C_INT index;

    if (config == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(config, 0, sizeof(*config));
    config->cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config->fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    if (options == STD_NULL) return NTVDM64_STATUS_OK;
    for (index = 0; index < options->argument_count; index += 2) {
        if (index + 1 >= options->argument_count || options->arguments == STD_NULL) {
            return NTVDM64_STATUS_INVALID_ARGUMENT;
        }
        if (!STD_STRCMP(options->arguments[index], "--cpu")) {
            if (!vm_session_provider_parse_cpu(options->arguments[index + 1],
                    &config->cpu_profile)) return NTVDM64_STATUS_INVALID_ARGUMENT;
        } else if (!STD_STRCMP(options->arguments[index], "--fpu")) {
            if (!vm_session_provider_parse_fpu(options->arguments[index + 1],
                    &config->fpu_profile)) return NTVDM64_STATUS_INVALID_ARGUMENT;
        } else {
            return NTVDM64_STATUS_INVALID_ARGUMENT;
        }
    }
    return config->fpu_profile == CORE_MACHINE_FPU_PROFILE_NONE ?
        NTVDM64_STATUS_OK : NTVDM64_STATUS_INVALID_STATE;
}

static ntvdm64_status vm_session_provider_open(C_VOID *context,
    core_product_session_id id, const core_product_session_open_options *options,
    C_VOID **out_session)
{
    vm_session *session;
    ntvdm64_status status;
    vm_session_config config;

    (C_VOID)context;
    status = vm_session_provider_parse_options(options, &config);
    if (status != NTVDM64_STATUS_OK) return status;
    status = vm_session_create(&config, &session);
    if (status != NTVDM64_STATUS_OK) return status;
    if (id == 0u) {
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
        vm_machine_fdd_insert_for(session->fdd, "d:/fd.img");
        vm_machine_hdd_insert(session->hdd, "d:/hd.img");
#else
        vm_machine_fdd_insert_for(session->fdd, "/Users/xha/fd.img");
        vm_machine_hdd_insert(session->hdd, "/Users/xha/hd.img");
#endif
    }
    *out_session = session;
    return NTVDM64_STATUS_OK;
}

static ntvdm64_status vm_session_provider_describe(C_VOID *context,
    const C_VOID *opaque, core_product_session_snapshot *snapshot)
{
    const vm_session *session = (const vm_session *)opaque;

    (C_VOID)context;
    if (session == STD_NULL || snapshot == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    snapshot->state = vm_session_control_is_running(session->control) ?
        (vm_session_control_is_paused(session->control) ?
            CORE_PRODUCT_SESSION_STATE_PAUSED : CORE_PRODUCT_SESSION_STATE_RUNNING) :
        CORE_PRODUCT_SESSION_STATE_STOPPED;
    snapshot->display = vm_platform_run_context_get_window_display(
        session->platform_run_context) ? CORE_PRODUCT_SESSION_DISPLAY_WINDOW :
        CORE_PRODUCT_SESSION_DISPLAY_CONSOLE;
    STD_SNPRINTF(snapshot->details, sizeof(snapshot->details), "cpu=%s fpu=%s",
        core_machine_cpu_profile_name(session->core_machine_config.cpu_profile),
        core_machine_fpu_profile_name(session->core_machine_config.fpu_profile));
    return NTVDM64_STATUS_OK;
}

static ntvdm64_status vm_session_provider_close(C_VOID *context, C_VOID *opaque)
{
    (C_VOID)context;
    vm_session_destroy((vm_session *)opaque);
    return NTVDM64_STATUS_OK;
}

C_VOID vm_session_provider_initialize(core_product_session_provider *provider)
{
    if (provider == STD_NULL) return;
    provider->open = vm_session_provider_open;
    provider->describe = vm_session_provider_describe;
    provider->close = vm_session_provider_close;
    provider->context = STD_NULL;
}

static C_VOID vm_session_machine_initialize(C_VOID *context) { (C_VOID)context; }
static C_VOID vm_session_machine_finalize(C_VOID *context) { (C_VOID)context; }
static C_INT vm_session_machine_is_running(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL && vm_session_control_is_running(s->control); }
static C_VOID vm_session_machine_print(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_print_machine(s); }
static C_INT vm_session_machine_get_window(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL && vm_platform_run_context_get_window_display(s->platform_run_context); }
static C_VOID vm_session_machine_set_window(C_VOID *context, C_INT value) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_platform_run_context_set_window_display(s->platform_run_context, value); }
static C_VOID vm_session_machine_print_bios(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_profile_default_bios_print(s->default_bios); }
static C_VOID vm_session_machine_print_status(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_control_print_status(s->control); }
static C_VOID vm_session_machine_debug(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); static const core_product_debug_input_provider input = { vm_session_debug_flush_console_input, STD_NULL }; if (s == STD_NULL) return; if (vm_session_control_is_running(s->control)) { vm_session_control_request_pause(s->control, VM_SESSION_PAUSE_EXPLICIT); if (!vm_session_control_wait_for_pause(s->control, 2000u)) return; } core_product_debug_context_initialize(s->debugger_context); s->debugger_context->wait_scope = s->wait_scope; core_product_debug_main(s->debugger_context, vm_session_debug_target(s), &input); }
static C_VOID vm_session_machine_record_start(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_machine_debug_record_start(s->debug, path); }
static C_VOID vm_session_machine_record_stop(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_machine_debug_record_stop(s->debug); }
static C_VOID vm_session_machine_boot_hdd(C_VOID *context, C_INT value) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_profile_default_bios_set_boot_hdd(s->default_bios, value); }
static C_VOID vm_session_machine_memory(C_VOID *context, STD_SIZE_T bytes) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) core_machine_memory_allocate_for(core_machine_executor_memory_borrow(s->core_machine), bytes); }
static C_VOID vm_session_machine_create_fdd(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_machine_fdd_create_for(s->fdd); }
static C_INT vm_session_machine_insert_fdd(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL ? vm_machine_fdd_insert_for(s->fdd, path) : -1; }
static C_INT vm_session_machine_remove_fdd(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL ? vm_machine_fdd_remove_for(s->fdd, path) : -1; }
static C_VOID vm_session_machine_create_hdd(C_VOID *context, uint16_t c) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_machine_hdd_create(s->hdd, c); }
static C_INT vm_session_machine_insert_hdd(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL ? vm_machine_hdd_insert(s->hdd, path) : -1; }
static C_INT vm_session_machine_remove_hdd(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL ? vm_machine_hdd_remove(s->hdd, path) : -1; }
static C_VOID vm_session_machine_start(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_start(s); }
static C_VOID vm_session_machine_reset(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_reset(s); }
static C_VOID vm_session_machine_stop(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_stop(s); }
static C_VOID vm_session_machine_resume(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_resume(s); }

static const vm_product_console_machine_provider vmSessionMachineProviderTemplate = {
    vm_session_machine_initialize, vm_session_machine_finalize,
    vm_session_machine_is_running, vm_session_machine_print,
    vm_session_machine_get_window, vm_session_machine_set_window,
    vm_session_machine_print_bios, vm_session_machine_print_status,
    vm_session_machine_debug, vm_session_machine_record_start,
    vm_session_machine_record_stop, vm_session_machine_boot_hdd,
    vm_session_machine_memory, vm_session_machine_create_fdd,
    vm_session_machine_insert_fdd, vm_session_machine_remove_fdd,
    vm_session_machine_create_hdd, vm_session_machine_insert_hdd,
    vm_session_machine_remove_hdd, vm_session_machine_start,
    vm_session_machine_reset, vm_session_machine_stop,
    vm_session_machine_resume, STD_NULL
};

C_VOID vm_session_machine_provider_initialize(
    vm_product_console_machine_provider *machine_provider,
    core_product_session_manager *manager)
{
    if (machine_provider == STD_NULL) return;
    *machine_provider = vmSessionMachineProviderTemplate;
    machine_provider->context = manager;
}
