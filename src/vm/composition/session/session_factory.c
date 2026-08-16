#include "type.h"

#include "vm/composition/session/session.h"

#include "vm/composition/session/provider.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/debug_target.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/machine_info.h"
#include "core/machine/memory.h"
#include "core/product/debug/debug.h"
#include "core/product/utils.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"
#include "vm/profile/default_profile/firmware/bios.h"

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

static C_INT vm_session_provider_parse_profile(const C_CHAR *value,
    vm_session_profile_kind *out_profile)
{
    if (value == STD_NULL || out_profile == STD_NULL) return 0;
    if (!STD_STRCMP(value, "1") || !STD_STRCMP(value, "default-pc-at")) {
        *out_profile = VM_SESSION_PROFILE_DEFAULT_PC_AT;
    } else if (!STD_STRCMP(value, "2") ||
        !STD_STRCMP(value, "ibm-5170-model-339")) {
        *out_profile = VM_SESSION_PROFILE_IBM_5170_MODEL_339;
    } else {
        return 0;
    }
    return 1;
}

static type_status vm_session_provider_parse_options(
    const core_product_session_open_options *options, vm_session_config *config)
{
    C_INT index;
    C_INT has_generic_override = 0;

    if (config == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(config, 0, sizeof(*config));
    config->cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config->fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    if (options == STD_NULL) return TYPE_STATUS_OK;
    for (index = 0; index < options->argument_count; index += 2) {
        if (index + 1 >= options->argument_count || options->arguments == STD_NULL) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        if (!STD_STRCMP(options->arguments[index], "--profile")) {
            if (!vm_session_provider_parse_profile(options->arguments[index + 1],
                    &config->profile_kind)) return TYPE_STATUS_INVALID_ARGUMENT;
        } else if (!STD_STRCMP(options->arguments[index], "--cpu")) {
            if (!vm_session_provider_parse_cpu(options->arguments[index + 1],
                    &config->cpu_profile)) return TYPE_STATUS_INVALID_ARGUMENT;
            has_generic_override = 1;
        } else if (!STD_STRCMP(options->arguments[index], "--fpu")) {
            if (!vm_session_provider_parse_fpu(options->arguments[index + 1],
                    &config->fpu_profile)) return TYPE_STATUS_INVALID_ARGUMENT;
            has_generic_override = 1;
        } else if (!STD_STRCMP(options->arguments[index], "--fdd")) {
            config->fdd_image = !STD_STRCMP(options->arguments[index + 1], "null") ?
                STD_NULL : options->arguments[index + 1];
        } else if (!STD_STRCMP(options->arguments[index], "--hdd")) {
            config->hdd_image = !STD_STRCMP(options->arguments[index + 1], "null") ?
                STD_NULL : options->arguments[index + 1];
        } else if (!STD_STRCMP(options->arguments[index], "--boot")) {
            if (!STD_STRCMP(options->arguments[index + 1], "hard_disk")) {
                config->boot_hdd = 1;
            } else if (STD_STRCMP(options->arguments[index + 1], "floppy") &&
                STD_STRCMP(options->arguments[index + 1], "rom")) {
                return TYPE_STATUS_INVALID_ARGUMENT;
            }
        } else if (!STD_STRCMP(options->arguments[index], "--display")) {
            if (STD_STRCMP(options->arguments[index + 1], "console") &&
                STD_STRCMP(options->arguments[index + 1], "window") &&
                STD_STRCMP(options->arguments[index + 1], "auto")) {
                return TYPE_STATUS_INVALID_ARGUMENT;
            }
        } else if (!STD_STRCMP(options->arguments[index], "--memory-kib")) {
            if (core_product_utils_parse_memory_kib(options->arguments[index + 1],
                    &config->memory_bytes) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_ARGUMENT;
            has_generic_override = 1;
        } else {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    }
    if (config->profile_kind != VM_SESSION_PROFILE_DEFAULT_PC_AT &&
        has_generic_override) return TYPE_STATUS_INVALID_STATE;
    return config->fpu_profile == CORE_MACHINE_FPU_PROFILE_NONE ?
        TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

static type_status vm_session_provider_open(C_VOID *context,
    core_product_session_id id, const core_product_session_open_options *options,
    C_VOID **out_session)
{
    vm_session *session;
    type_status status;
    vm_session_config config;

    (C_VOID)context;
    status = vm_session_provider_parse_options(options, &config);
    if (status != TYPE_STATUS_OK) return status;
    status = vm_session_create(&config, &session);
    if (status != TYPE_STATUS_OK) return status;
    (C_VOID)id;
    *out_session = session;
    return TYPE_STATUS_OK;
}

static type_status vm_session_provider_describe(C_VOID *context,
    const C_VOID *opaque, core_product_session_snapshot *snapshot)
{
    const vm_session *session = (const vm_session *)opaque;

    (C_VOID)context;
    if (session == STD_NULL || snapshot == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    snapshot->state = vm_session_control_is_running(&session->control) ?
        (vm_session_control_is_paused(&session->control) ?
            CORE_PRODUCT_SESSION_STATE_PAUSED : CORE_PRODUCT_SESSION_STATE_RUNNING) :
        CORE_PRODUCT_SESSION_STATE_STOPPED;
    snapshot->display = vm_platform_run_context_get_window_display(
        &session->platform_run_context) ? CORE_PRODUCT_SESSION_DISPLAY_WINDOW :
        CORE_PRODUCT_SESSION_DISPLAY_CONSOLE;
    STD_SNPRINTF(snapshot->details, sizeof(snapshot->details),
        "profile=%s cpu=%s fpu=%s",
        vm_session_profile_name(session->retained_config.profile_kind),
        core_machine_cpu_profile_name(session->core_machine_config.cpu_profile),
        core_machine_fpu_profile_name(session->core_machine_config.fpu_profile));
    return TYPE_STATUS_OK;
}

static type_status vm_session_provider_close(C_VOID *context, C_VOID *opaque)
{
    (C_VOID)context;
    vm_session_destroy((vm_session *)opaque);
    return TYPE_STATUS_OK;
}

C_VOID vm_session_provider_initialize(core_product_session_provider *provider)
{
    if (provider == STD_NULL) return;
    provider->open = vm_session_provider_open;
    provider->describe = vm_session_provider_describe;
    provider->close = vm_session_provider_close;
    provider->context = STD_NULL;
}
