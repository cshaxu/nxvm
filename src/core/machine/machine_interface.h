#ifndef VM_MACHINE_INTERFACE_H
#define VM_MACHINE_INTERFACE_H

#include "type.h"

#include "core/devices/cpu_interface.h"
#include "core/devices/fpu_interface.h"
#include "core/devices/guest_input_interface.h"
#include "core/profiles/selection_interface.h"
#include "core/machine/event_interface.h"
#include "core/profiles/byob/blob.h"
#include "core/profiles/default_profile/external_pc_at_rom.h"
#include "common/machine/machine_interface.h"
#include "lib/storage/medium_interface.h"

typedef enum vm_machine_speed {
    VM_MACHINE_SPEED_STANDARD,
    VM_MACHINE_SPEED_TURBO
} vm_machine_speed;

const C_CHAR *vm_machine_profile_name(vm_machine_profile_kind kind);

typedef struct vm_machine vm_machine;

typedef struct vm_machine_reset_vector {
    type_unsigned_16 cs;
    type_unsigned_16 ip;
} vm_machine_reset_vector;

/* Copied monitor facts.  The product formats these facts; vm/machine never
 * writes monitor text or exposes its Core, media or control objects. */
typedef struct vm_machine_information {
    vm_machine_profile_kind profile_kind;
    core_machine_cpu_profile cpu_profile;
    STD_SIZE_T memory_bytes;
    STD_SIZE_T floppy_image_bytes;
    C_INT floppy_media_inserted;
    C_INT fixed_disk_present;
    type_unsigned_32 fixed_disk_cylinders;
    STD_SIZE_T fixed_disk_image_bytes;
    C_INT fixed_disk_media_connected;
    C_INT external_firmware;
    C_INT active;
    C_INT fault_valid;
    type_unsigned_32 fault_detail;
    type_unsigned_32 fault_linear_pc;
    C_INT fault_exception_valid;
    type_unsigned_32 fault_exception_mask;
    type_unsigned_32 fault_exception_code;
    type_unsigned_16 fault_exception_cs;
    type_unsigned_32 fault_exception_eip;
} vm_machine_information;

C_INT vm_machine_create(const vm_machine_config *config, vm_machine **out_session);
type_status vm_machine_create_from_assets(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_session);
C_VOID vm_machine_destroy(vm_machine *session);
/* vm/machine supplies this value-only driver; App composition owns the Common
 * machine it constructs from it. */
type_status vm_machine_describe_common_driver(vm_machine *session,
    common_machine_driver *out_driver);
/* App binds its Common owner before lifecycle requests.  Passing NULL revokes
 * that non-owning link during ordered teardown. */
type_status vm_machine_bind_common_machine(vm_machine *session,
    common_machine *common_machine);
type_status vm_machine_reconfigure_memory(vm_machine *session,
    STD_SIZE_T memory_bytes);
type_status vm_machine_get_speed(const vm_machine *session,
    vm_machine_speed *out_speed);
type_status vm_machine_set_speed(vm_machine *session, vm_machine_speed speed);
C_INT vm_machine_insert_fdd(vm_machine *session, const C_CHAR *path);
C_INT vm_machine_eject_fdd(vm_machine *session);
/* Production host-input ingress.  A composed session copies events into
 * Common's ordered executor transport; it never mutates a running guest
 * device from the host thread.  An uncomposed deterministic Core loop may
 * use the same API for owner-local tests. */
type_status vm_machine_submit_host_input(vm_machine *session,
    const core_machine_guest_input_event *event);
/* vm/app uses this value-only ingress; vm/machine alone translates it to
 * Common's neutral guest-input event. */
type_status vm_machine_submit_input(vm_machine *session,
    const vm_machine_input *input);
type_status vm_machine_get_reset_vector(const vm_machine *session,
    vm_machine_reset_vector *out_vector);
type_status vm_machine_get_information(const vm_machine *session,
    vm_machine_information *out_information);
C_INT vm_machine_is_running(const vm_machine *session);
#endif
