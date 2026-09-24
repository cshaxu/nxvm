#ifndef VM_MACHINE_INTERFACE_H
#define VM_MACHINE_INTERFACE_H
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/cpu_interface.h"
#include "app-nxvm/devices/fpu_interface.h"
#include "app-nxvm/devices/guest_input_interface.h"
#include "app-nxvm/profiles/selection_interface.h"
#include "app-nxvm/machine/event_interface.h"
#include "app-nxvm/profiles/byob/blob.h"
#include "app-nxvm/profiles/default_profile/external_pc_at_rom.h"
#include "common/machine/machine_interface.h"
#include "lib/storage/medium_interface.h"

typedef enum vm_machine_speed {
    VM_MACHINE_SPEED_STANDARD,
    VM_MACHINE_SPEED_TURBO
} vm_machine_speed;

typedef struct vm_machine vm_machine;

typedef struct vm_machine_reset_vector {
    lib_u16 cs;
    lib_u16 ip;
} vm_machine_reset_vector;

/* Copied monitor facts.  The product formats these facts; vm/machine never
 * writes monitor text or exposes its Core, media or control objects. */
typedef struct vm_machine_information {
    vm_machine_profile_kind profile_kind;
    core_machine_cpu_profile cpu_profile;
    lib_size memory_bytes;
    lib_size floppy_image_bytes;
    lib_i32 floppy_media_inserted;
    lib_i32 fixed_disk_present;
    lib_u32 fixed_disk_cylinders;
    lib_size fixed_disk_image_bytes;
    lib_i32 fixed_disk_media_connected;
    lib_i32 external_firmware;
    lib_i32 active;
    lib_i32 fault_valid;
    lib_u32 fault_detail;
    lib_u32 fault_linear_pc;
    lib_i32 fault_exception_valid;
    lib_u32 fault_exception_mask;
    lib_u32 fault_exception_code;
    lib_u16 fault_exception_cs;
    lib_u32 fault_exception_eip;
} vm_machine_information;

lib_status vm_machine_create(const vm_machine_config *config,
    vm_machine **out_session);
lib_status vm_machine_create_from_assets(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_session);
void vm_machine_destroy(vm_machine *session);
/* vm/machine supplies this value-only driver; App composition owns the Common
 * machine it constructs from it. */
lib_status vm_machine_describe_common_driver(vm_machine *session,
    common_machine_driver *out_driver);
/* App binds its Common owner before lifecycle requests.  Passing NULL revokes
 * that non-owning link during ordered teardown. */
lib_status vm_machine_bind_common_machine(vm_machine *session,
    common_machine *common_machine);
lib_status vm_machine_reconfigure_memory(vm_machine *session,
    lib_size memory_bytes);
lib_status vm_machine_get_speed(const vm_machine *session,
    vm_machine_speed *out_speed);
lib_status vm_machine_set_speed(vm_machine *session, vm_machine_speed speed);
lib_i32 vm_machine_insert_fdd(vm_machine *session, const char *path);
lib_i32 vm_machine_eject_fdd(vm_machine *session);
/* Production host-input ingress.  A composed session copies events into
 * Common's ordered executor transport; it never mutates a running guest
 * device from the host thread.  An uncomposed deterministic Core loop may
 * use the same API for owner-local tests. */
lib_status vm_machine_submit_host_input(vm_machine *session,
    const core_machine_guest_input_event *event);
/* vm/app uses this value-only ingress; vm/machine alone translates it to
 * Common's neutral guest-input event. */
lib_status vm_machine_submit_input(vm_machine *session,
    const vm_machine_input *input);
lib_status vm_machine_get_reset_vector(const vm_machine *session,
    vm_machine_reset_vector *out_vector);
lib_status vm_machine_get_information(const vm_machine *session,
    vm_machine_information *out_information);
#endif
