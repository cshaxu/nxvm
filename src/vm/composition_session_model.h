#ifndef NXVM_COMPOSITION_SESSION_MODEL_H
#define NXVM_COMPOSITION_SESSION_MODEL_H

#include "vm/profile/default_profile/firmware/default_profile.h"
#include "vm/product/session_console.h"
#include "vm/product/session_debugger.h"
#include "vm/product/media.h"
#include "vm/composition_default_profile.h"
#include "vm/product/presentation.h"
#include "core/product/runtime/registry.h"

typedef struct vm_composition_session_model_config {
    const char *fdd_path;
    const nxvm_product_nxvm_media_identity *fdd_identity;
    const char *hdd_path;
    const nxvm_product_nxvm_media_identity *hdd_identity;
    int create_fdd;
    uint16_t create_hdd_cylinders;
    nxvm_product_nxvm_boot_target boot_target;
} vm_composition_session_model_config;

typedef struct vm_composition_session_model {
    core_product_runtime_registry registry;
    core_machine_firmware firmware;
    vm_profile_default_firmware_plan firmware_plan;
    vm_profile_default_firmware_cmos cmos;
    nxvm_product_nxvm_media_policy media;
    nxvm_product_nxvm_default_profile default_profile;
    nxvm_product_nxvm_console console;
    nxvm_product_nxvm_presentation presentation;
    core_machine *firmware_machine;
    nxvm_product_nxvm_debugger debugger;
} vm_composition_session_model;

ntvdm64_status vm_composition_session_model_create(
    vm_composition_session_model *session,
    const vm_composition_session_model_config *config);
ntvdm64_status vm_composition_session_model_get_firmware_reset_vector(
    const vm_composition_session_model *session,
    nxvm_product_nxvm_reset_vector *out_vector);
ntvdm64_status vm_composition_session_model_get_execution_reset_vector(
    const vm_composition_session_model *session,
    nxvm_product_nxvm_reset_vector *out_vector);
void vm_composition_session_model_destroy(vm_composition_session_model *session);

#endif
