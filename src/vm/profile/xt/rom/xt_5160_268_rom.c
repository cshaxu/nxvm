#include "type.h"

#include "vm/profile/xt/xt_5160_268.h"
#include "vm/profile/byob/blob.h"

C_INT vm_profile_xt_5160_268_byob_manifest_is_valid(
    const vm_profile_xt_5160_268_byob_manifest *manifest)
{
    return manifest != STD_NULL && manifest->system_path != STD_NULL &&
        manifest->system_path[0] != '\0' &&
        ((manifest->system_sha256 == STD_NULL) || manifest->system_sha256[0] != '\0') &&
        ((manifest->xebec_path == STD_NULL && manifest->xebec_sha256 == STD_NULL) ||
         (manifest->xebec_path != STD_NULL && manifest->xebec_path[0] != '\0' &&
          (manifest->xebec_sha256 == STD_NULL || manifest->xebec_sha256[0] != '\0')));
}

type_status vm_profile_xt_5160_268_external_rom_create(
    const type_unsigned_8 *system, STD_SIZE_T system_bytes,
    const type_unsigned_8 *xebec, STD_SIZE_T xebec_bytes,
    const type_unsigned_8 *video, STD_SIZE_T video_bytes,
    vm_profile_xt_5160_268_external_rom *out_rom)
{
    if (out_rom == STD_NULL || system == STD_NULL ||
        system_bytes != VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES ||
        (xebec == STD_NULL && xebec_bytes != 0u) ||
        (xebec != STD_NULL && xebec_bytes != VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES) ||
        (video == STD_NULL && video_bytes != 0u) ||
        (video != STD_NULL && !vm_profile_byob_option_rom_is_valid(video,
            video_bytes, VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES))) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_rom = (vm_profile_xt_5160_268_external_rom) { system, xebec, video,
        video == STD_NULL ? 0u : video_bytes, xebec != STD_NULL };
    return TYPE_STATUS_OK;
}

type_status vm_profile_xt_5160_268_byob_manifest_load(
    const vm_profile_xt_5160_268_byob_manifest *manifest,
    type_unsigned_8 *system_bytes, type_unsigned_8 *xebec_bytes,
    vm_profile_xt_5160_268_external_rom *out_rom)
{
    type_status status;

    if (!vm_profile_xt_5160_268_byob_manifest_is_valid(manifest) ||
        system_bytes == STD_NULL || xebec_bytes == STD_NULL || out_rom == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {manifest->system_path,
        manifest->system_sha256, VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES}, system_bytes);
    if (status != TYPE_STATUS_OK) return status;
    if (manifest->xebec_path != STD_NULL) {
        status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {manifest->xebec_path,
            manifest->xebec_sha256, VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES}, xebec_bytes);
        if (status != TYPE_STATUS_OK) {
            STD_MEMSET(system_bytes, 0, VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
            return status;
        }
    }
    return vm_profile_xt_5160_268_external_rom_create(system_bytes,
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES,
        manifest->xebec_path == STD_NULL ? STD_NULL : xebec_bytes,
        manifest->xebec_path == STD_NULL ? 0u : VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES,
        STD_NULL, 0u,
        out_rom);
}

static type_status vm_profile_xt_5160_268_firmware_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_xt_5160_268_external_rom *rom = opaque;
    type_status status;

    if (rom == STD_NULL || rom->system_bytes == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_firmware_register_immutable_rom(firmware,
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_PHYSICAL_START, rom->system_bytes,
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    if (status != TYPE_STATUS_OK) return status;
    if (rom->video_bytes != STD_NULL) {
        status = core_machine_firmware_register_immutable_rom(firmware,
            0x000c0000u, rom->video_bytes, rom->video_byte_count);
        if (status != TYPE_STATUS_OK) return status;
    }
    if (!rom->xebec_present) return status;
    if (rom->xebec_bytes == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return core_machine_firmware_register_immutable_rom(firmware,
        VM_PROFILE_XT_5160_268_XEBEC_ROM_PHYSICAL_START, rom->xebec_bytes,
        VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
}

static type_status vm_profile_xt_5160_268_firmware_reset(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_xt_5160_268_external_rom *rom = opaque;

    (C_VOID)firmware;
    return rom == STD_NULL || rom->system_bytes == STD_NULL ||
        (rom->xebec_present && rom->xebec_bytes == STD_NULL) ||
        (rom->video_bytes != STD_NULL && !vm_profile_byob_option_rom_is_valid(
            rom->video_bytes, rom->video_byte_count,
            VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES)) ?
        TYPE_STATUS_INVALID_ARGUMENT : TYPE_STATUS_OK;
}

static const core_machine_firmware_provider vm_profile_xt_5160_268_provider = {
    vm_profile_xt_5160_268_firmware_configure,
    vm_profile_xt_5160_268_firmware_reset,
    STD_NULL,
    STD_NULL
};

const core_machine_firmware_provider *vm_profile_xt_5160_268_firmware_provider(C_VOID)
{
    return &vm_profile_xt_5160_268_provider;
}
