/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_FIRMWARE_PORTAL_H
#define VM_PROFILE_DEFAULT_FIRMWARE_PORTAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/machine_interface.h"

typedef struct vm_profile_default_context vm_profile_default_context;

#define VM_PROFILE_DEFAULT_PORTAL_STOP            0xf0u
#define VM_PROFILE_DEFAULT_PORTAL_KEYBOARD_IRQ1   0xf1u
#define VM_PROFILE_DEFAULT_PORTAL_VIDEO_INT10     0xf2u
#define VM_PROFILE_DEFAULT_PORTAL_KEYBOARD_INT16  0xf3u
#define VM_PROFILE_DEFAULT_PORTAL_HDD_READ        0xf4u
#define VM_PROFILE_DEFAULT_PORTAL_HDD_WRITE       0xf5u
#define VM_PROFILE_DEFAULT_PORTAL_ORIGIN_LINEAR_START 0x000f0000u
#define VM_PROFILE_DEFAULT_PORTAL_ORIGIN_LINEAR_BYTES 0x00010000u

type_status vm_profile_default_firmware_portal_install(core_machine *machine,
    vm_profile_default_context *profile, uint32_t origin_linear_start,
    uint32_t origin_linear_bytes);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
