#ifndef NXVM_COMPOSITION_SESSION_MODEL_H
#define NXVM_COMPOSITION_SESSION_MODEL_H

#include "type.h"

#include "vm/profile/default_profile/firmware/default_profile.h"

#include "vm/product/session_console.h"

#include "vm/product/session_debugger.h"

#include "vm/product/media.h"

#include "vm/composition/composition_default_profile.h"

#include "vm/product/presentation.h"

#include "core/product/runtime/registry.h"

typedef struct vm_composition_session_model_config {
    const C_CHAR *fdd_path;
    const vm_product_media_identity *fdd_identity;
    const C_CHAR *hdd_path;
    const vm_product_media_identity *hdd_identity;
    C_INT create_fdd;
    uint16_t create_hdd_cylinders;
    vm_product_boot_target boot_target;
} vm_composition_session_model_config;

typedef struct vm_composition_session_model {
    core_product_runtime_registry registry;
    core_machine_firmware firmware;
    vm_profile_default_firmware_plan firmware_plan;
    vm_profile_default_firmware_cmos cmos;
    vm_product_media_policy media;
    vm_composition_default_profile default_profile;
    vm_product_console console;
    vm_product_presentation presentation;
    core_machine *firmware_machine;
    vm_product_debugger debugger;
} vm_composition_session_model;

ntvdm64_status vm_composition_session_model_create(
    vm_composition_session_model *session,
    const vm_composition_session_model_config *config);
ntvdm64_status vm_composition_session_model_get_firmware_reset_vector(
    const vm_composition_session_model *session,
    vm_composition_default_profile_reset_vector *out_vector);
ntvdm64_status vm_composition_session_model_get_execution_reset_vector(
    const vm_composition_session_model *session,
    vm_composition_default_profile_reset_vector *out_vector);
C_VOID vm_composition_session_model_destroy(vm_composition_session_model *session);

#endif
