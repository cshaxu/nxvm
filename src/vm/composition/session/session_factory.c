#include "type.h"

#include "vm/composition/session/session_private.h"

#include "vm/composition/session/provider.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/debug_target.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/machine_info.h"
#include "core/product/debug/debug.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/product/session_catalog.h"

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

static C_INT vm_session_provider_parse_floppy_format(const C_CHAR *value,
    vm_session_floppy_format *out_format)
{
    if (!STD_STRCMP(value, "360k")) *out_format = VM_SESSION_FLOPPY_FORMAT_360K;
    else if (!STD_STRCMP(value, "720k")) *out_format = VM_SESSION_FLOPPY_FORMAT_720K;
    else if (!STD_STRCMP(value, "1200k")) *out_format = VM_SESSION_FLOPPY_FORMAT_1200K;
    else if (!STD_STRCMP(value, "1440k")) *out_format = VM_SESSION_FLOPPY_FORMAT_1440K;
    else return 0;
    return 1;
}

static C_INT vm_session_provider_parse_profile(const C_CHAR *value,
    vm_session_profile_kind *out_profile)
{
    if (value == STD_NULL || out_profile == STD_NULL) return 0;
    if (!STD_STRCMP(value, "default-pc-at")) {
        *out_profile = VM_SESSION_PROFILE_DEFAULT_PC_AT;
    } else if (!STD_STRCMP(value, "ibm-5170-model-339")) {
        *out_profile = VM_SESSION_PROFILE_IBM_5170_MODEL_339;
    } else if (!STD_STRCMP(value, "ibm-5160-model-268")) {
        *out_profile = VM_SESSION_PROFILE_IBM_5160_MODEL_268;
    } else if (!STD_STRCMP(value, "compaq-deskpro-386-model-40")) {
        *out_profile = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    } else {
        return 0;
    }
    return 1;
}

static type_status vm_session_provider_request_configure(
    const vm_product_session_request *request, vm_session_config *config)
{
    if (request == STD_NULL || config == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if ((STD_STRCMP(request->display, "console") &&
         STD_STRCMP(request->display, "window") &&
         STD_STRCMP(request->display, "auto")) ||
        (STD_STRCMP(request->boot, "floppy") &&
         STD_STRCMP(request->boot, "hard_disk") &&
         STD_STRCMP(request->boot, "rom"))) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(config, 0, sizeof(*config));
    if (!vm_session_provider_parse_profile(request->profile, &config->profile_kind)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    config->cpu_profile = CORE_MACHINE_CPU_PROFILE_DEFAULT;
    config->fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    if ((request->cpu[0] != '\0' && !vm_session_provider_parse_cpu(request->cpu,
            &config->cpu_profile)) ||
        (request->fpu[0] != '\0' && !vm_session_provider_parse_fpu(request->fpu,
            &config->fpu_profile))) return TYPE_STATUS_INVALID_ARGUMENT;
    if (request->floppy_format[0] != '\0' &&
        !vm_session_provider_parse_floppy_format(request->floppy_format,
            &config->floppy_format)) return TYPE_STATUS_INVALID_ARGUMENT;
    config->memory_bytes = request->memory_bytes;
    config->fdd_image = request->floppy[0] == '\0' ? STD_NULL : request->floppy;
    config->hdd_image = request->hard_disk[0] == '\0' ? STD_NULL : request->hard_disk;
    config->boot_hdd = !STD_STRCMP(request->boot, "hard_disk");
    if (config->profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        config->model40_firmware.even_path = request->model40_even_path;
        config->model40_firmware.even_sha256 = request->model40_even_sha256;
        config->model40_firmware.odd_path = request->model40_odd_path;
        config->model40_firmware.odd_sha256 = request->model40_odd_sha256;
        config->model40_firmware.provenance = request->model40_provenance;
    } else if (config->profile_kind == VM_SESSION_PROFILE_IBM_5160_MODEL_268) {
        config->xt_firmware.system_path = request->xt_system_path;
        config->xt_firmware.system_sha256 = request->xt_system_sha256;
        config->xt_firmware.xebec_path = request->xt_xebec_path[0] == '\0' ?
            STD_NULL : request->xt_xebec_path;
        config->xt_firmware.xebec_sha256 = request->xt_xebec_sha256[0] == '\0' ?
            STD_NULL : request->xt_xebec_sha256;
        config->xt_firmware.provenance = request->xt_provenance;
    }
    if (config->profile_kind != VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40 &&
        (request->model40_even_path[0] != '\0' || request->model40_even_sha256[0] != '\0' ||
         request->model40_odd_path[0] != '\0' || request->model40_odd_sha256[0] != '\0' ||
         request->model40_provenance[0] != '\0')) return TYPE_STATUS_INVALID_STATE;
    if (config->profile_kind != VM_SESSION_PROFILE_IBM_5160_MODEL_268 &&
        (request->xt_system_path[0] != '\0' || request->xt_system_sha256[0] != '\0' ||
         request->xt_xebec_path[0] != '\0' || request->xt_xebec_sha256[0] != '\0' ||
         request->xt_provenance[0] != '\0')) return TYPE_STATUS_INVALID_STATE;
    if (config->profile_kind != VM_SESSION_PROFILE_DEFAULT_PC_AT &&
        (config->memory_bytes != 0u || request->cpu[0] != '\0' ||
         request->fpu[0] != '\0' || request->floppy_format[0] != '\0')) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (config->profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40 &&
        STD_STRCMP(request->boot, "rom")) return TYPE_STATUS_INVALID_STATE;
    if (config->profile_kind == VM_SESSION_PROFILE_IBM_5160_MODEL_268 &&
        (STD_STRCMP(request->boot, "rom") || !vm_profile_xt_5160_268_byob_manifest_is_valid(
            &config->xt_firmware))) return TYPE_STATUS_INVALID_STATE;
    if ((!STD_STRCMP(request->boot, "floppy") && config->fdd_image == STD_NULL) ||
        (!STD_STRCMP(request->boot, "hard_disk") && config->hdd_image == STD_NULL) ||
        (config->profile_kind == VM_SESSION_PROFILE_IBM_5170_MODEL_339 &&
         config->hdd_image != STD_NULL)) return TYPE_STATUS_INVALID_STATE;
    if (config->profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40 &&
        !vm_profile_model40_byob_manifest_is_valid(&config->model40_firmware)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (config->profile_kind != VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40 &&
        (config->model40_firmware.even_path != STD_NULL ||
         config->model40_firmware.even_sha256 != STD_NULL ||
         config->model40_firmware.odd_path != STD_NULL ||
         config->model40_firmware.odd_sha256 != STD_NULL ||
         config->model40_firmware.provenance != STD_NULL)) return TYPE_STATUS_INVALID_STATE;
    if (config->profile_kind != VM_SESSION_PROFILE_IBM_5160_MODEL_268 &&
        (config->xt_firmware.system_path != STD_NULL || config->xt_firmware.system_sha256 != STD_NULL ||
         config->xt_firmware.xebec_path != STD_NULL || config->xt_firmware.xebec_sha256 != STD_NULL ||
         config->xt_firmware.provenance != STD_NULL)) return TYPE_STATUS_INVALID_STATE;
    return TYPE_STATUS_OK;
}

static type_status vm_session_provider_parse_options(
    const core_product_session_open_options *options, vm_session_config *config)
{
    if (config == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(config, 0, sizeof(*config));
    config->cpu_profile = CORE_MACHINE_CPU_PROFILE_DEFAULT;
    config->fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    if (options == STD_NULL || (options->argument_count == 0 &&
            options->request == STD_NULL && options->request_bytes == 0u)) {
        return TYPE_STATUS_OK;
    }
    if (options->argument_count != 0 || options->arguments != STD_NULL ||
        options->request == STD_NULL ||
        options->request_bytes != sizeof(vm_product_session_request)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return vm_session_provider_request_configure(
        (const vm_product_session_request *)options->request, config);
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
        session->platform_run_context) ? CORE_PRODUCT_SESSION_DISPLAY_WINDOW :
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
