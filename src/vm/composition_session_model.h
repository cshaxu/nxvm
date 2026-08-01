#ifndef NXVM_COMPOSITION_SESSION_MODEL_H
#define NXVM_COMPOSITION_SESSION_MODEL_H

#include "vm/profile/default_profile/firmware/default_profile.h"
#include "vm/product/session_console.h"
#include "vm/product/session_debugger.h"
#include "vm/product/media.h"
#include "vm/composition_default_profile.h"
#include "vm/product/presentation.h"
#include "core/product/runtime/registry.h"

typedef struct nxvm_product_nxvm_session_config {
    const char *fdd_path;
    const nxvm_product_nxvm_media_identity *fdd_identity;
    const char *hdd_path;
    const nxvm_product_nxvm_media_identity *hdd_identity;
    int create_fdd;
    uint16_t create_hdd_cylinders;
    nxvm_product_nxvm_boot_target boot_target;
} nxvm_product_nxvm_session_config;

typedef struct nxvm_product_nxvm_session {
    nxvm_runtime_registry registry;
    nxvm_firmware firmware;
    nxvm_firmware_default_profile_plan firmware_plan;
    nxvm_firmware_default_profile_cmos cmos;
    nxvm_product_nxvm_media_policy media;
    nxvm_product_nxvm_default_profile default_profile;
    nxvm_product_nxvm_console console;
    nxvm_product_nxvm_presentation presentation;
    nxvm_core_machine *firmware_machine;
    nxvm_product_nxvm_debugger debugger;
} nxvm_product_nxvm_session;

nxvm_core_status nxvm_product_nxvm_session_create(
    nxvm_product_nxvm_session *session,
    const nxvm_product_nxvm_session_config *config);
nxvm_core_status nxvm_product_nxvm_session_get_firmware_reset_vector(
    const nxvm_product_nxvm_session *session,
    nxvm_product_nxvm_reset_vector *out_vector);
nxvm_core_status nxvm_product_nxvm_session_get_execution_reset_vector(
    const nxvm_product_nxvm_session *session,
    nxvm_product_nxvm_reset_vector *out_vector);
void nxvm_product_nxvm_session_destroy(nxvm_product_nxvm_session *session);

#endif
