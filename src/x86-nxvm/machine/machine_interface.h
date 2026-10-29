#ifndef X86_NXVM_MACHINE_INTERFACE_H
#define X86_NXVM_MACHINE_INTERFACE_H
#include "lib/types/types_interface.h"


#include "x86-nxvm/machine/input_interface.h"
#include "x86-nxvm/board-base/rom_validation_interface.h"
#include "emulator/machine/machine_interface.h"
#include "lib/storage/medium_interface.h"
#include "x86-nxvm/machine/speed_interface.h"

typedef struct x86_nxvm_machine x86_nxvm_machine;

typedef struct x86_nxvm_machine_reset_vector {
    lib_u16 cs;
    lib_u16 ip;
} x86_nxvm_machine_reset_vector;

/* Copied monitor facts.  The product formats these facts; x86-nxvm/machine never
 * writes monitor text or exposes its Core, media or control objects. */
typedef struct x86_nxvm_machine_information {
    x86_core_cpu_profile cpu_profile;
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
} x86_nxvm_machine_information;

/* With three non-LIB_NULL arguments, construction.profile ownership transfers
 * on entry, including failure. Missing arguments do not transfer ownership.
 * The output stays LIB_NULL until the full creation/reset transaction succeeds. */
lib_status x86_nxvm_machine_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_construction *construction, x86_nxvm_machine **out_session);
void x86_nxvm_machine_destroy(x86_nxvm_machine *session);
/* x86-nxvm/machine supplies this value-only driver; App composition owns the Common
 * machine it constructs from it. */
lib_status x86_nxvm_machine_describe_emulator_driver(x86_nxvm_machine *session,
    emulator_machine_driver *out_driver);
/* App binds its Common owner before lifecycle requests.  Passing LIB_NULL revokes
 * that non-owning link during ordered teardown. */
lib_status x86_nxvm_machine_bind_emulator_machine(x86_nxvm_machine *session,
    emulator_machine *emulator_machine);
/* Caller ensures quiescence; a bound Emulator machine is rejected. Bytes
 * 00h--0Dh are zero; the live RTC retains ownership of those registers. */
lib_status x86_nxvm_machine_capture_cmos(x86_nxvm_machine *session,
    lib_u8 *bytes, lib_size byte_count);
/* Capture RTC register values at the same unbound, quiescent boundary. */
lib_status x86_nxvm_machine_capture_rtc(x86_nxvm_machine *session,
    lib_u8 *bytes, lib_size byte_count);
lib_status x86_nxvm_machine_reconfigure_memory(x86_nxvm_machine *session,
    lib_size memory_bytes);
lib_status x86_nxvm_machine_get_speed(const x86_nxvm_machine *session,
    x86_nxvm_machine_speed *out_speed);
lib_status x86_nxvm_machine_set_speed(x86_nxvm_machine *session, x86_nxvm_machine_speed speed);
lib_i32 x86_nxvm_machine_insert_fdd(x86_nxvm_machine *session, const char *path);
lib_i32 x86_nxvm_machine_eject_fdd(x86_nxvm_machine *session);
lib_status x86_nxvm_machine_get_reset_vector(const x86_nxvm_machine *session,
    x86_nxvm_machine_reset_vector *out_vector);
lib_status x86_nxvm_machine_get_information(const x86_nxvm_machine *session,
    x86_nxvm_machine_information *out_information);
#endif
