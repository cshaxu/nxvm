#ifndef VM_MACHINE_INTERFACE_H
#define VM_MACHINE_INTERFACE_H
#include "lib/types/types_interface.h"


#include "core/machine/input_interface.h"
#include "core/board-base/rom_validation_interface.h"
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
    core_machine_cpu_profile cpu_profile;
    lib_size memory_bytes;
    lib_size floppy_image_bytes;
    lib_bool floppy_media_inserted;
    lib_bool fixed_disk_present;
    lib_u32 fixed_disk_cylinders;
    lib_size fixed_disk_image_bytes;
    lib_bool fixed_disk_media_connected;
    lib_bool external_firmware;
    lib_bool active;
    lib_bool fault_valid;
    lib_u32 fault_detail;
    lib_u32 fault_linear_pc;
    lib_bool fault_exception_valid;
    lib_u32 fault_exception_mask;
    lib_u32 fault_exception_code;
    lib_u16 fault_exception_cs;
    lib_u32 fault_exception_eip;
} vm_machine_information;

/* With three non-NULL arguments, construction.profile ownership transfers
 * on entry, including failure. Missing arguments do not transfer ownership.
 * The output stays NULL until the full creation/reset transaction succeeds. */
lib_status vm_machine_create(const vm_machine_config *config,
    const vm_machine_construction *construction, vm_machine **out_session);
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
lib_status vm_machine_get_reset_vector(const vm_machine *session,
    vm_machine_reset_vector *out_vector);
lib_status vm_machine_get_information(const vm_machine *session,
    vm_machine_information *out_information);
#endif
