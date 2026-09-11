#ifndef VM_MACHINE_INTERFACE_H
#define VM_MACHINE_INTERFACE_H

#include "type.h"

#include "core/machine/cpu_interface.h"
#include "core/machine/fpu_interface.h"
#include "core/machine/guest_input_interface.h"
#include "vm/events/machine_event.h"
#include "vm/profile/byob/blob.h"

typedef enum vm_machine_profile_kind {
    VM_MACHINE_PROFILE_DEFAULT_PC_AT,
    VM_MACHINE_PROFILE_IBM_5170_MODEL_339,
    VM_MACHINE_PROFILE_IBM_5160_MODEL_268,
    VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
} vm_machine_profile_kind;

typedef enum vm_machine_speed {
    VM_MACHINE_SPEED_STANDARD,
    VM_MACHINE_SPEED_TURBO
} vm_machine_speed;

/* A session selects the physical drive/media format at construction.  This is
 * not inferred from the image; the selected FDD remains the sole geometry
 * owner and rejects a differently sized image. */
typedef enum vm_machine_floppy_format {
    VM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT,
    VM_MACHINE_FLOPPY_FORMAT_360K,
    VM_MACHINE_FLOPPY_FORMAT_720K,
    VM_MACHINE_FLOPPY_FORMAT_1200K,
    VM_MACHINE_FLOPPY_FORMAT_1440K
} vm_machine_floppy_format;

#define VM_MACHINE_FLOPPY_SLOT_COUNT 2u
#define VM_MACHINE_FIXED_DISK_SLOT_COUNT 2u
#define VM_MACHINE_CMOS_SEED_BYTES 64u
#define VM_MACHINE_PC_AT_ROM_BYTES (64u * 1024u)
#define VM_MACHINE_PC_AT_ROM_CHIP_BYTES (32u * 1024u)
#define VM_MACHINE_PC_AT_VIDEO_ROM_MAX_BYTES VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES
#define VM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES (2u * 1024u)
#define VM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES \
    (4u * VM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES)

const C_CHAR *vm_machine_profile_name(vm_machine_profile_kind kind);

typedef struct vm_machine_config {
    vm_machine_profile_kind profile_kind;
    STD_SIZE_T memory_bytes;
    const C_CHAR *floppy_image[VM_MACHINE_FLOPPY_SLOT_COUNT];
    const C_CHAR *fixed_disk_image[VM_MACHINE_FIXED_DISK_SLOT_COUNT];
    /* Optional profile-scoped MC146818 NVRAM seed.  It is copied once during
     * construction; Core remains the sole owner of the live CMOS state. */
    const C_CHAR *cmos_seed;
    const C_CHAR *font_path;
    /* Generic PC/AT system-ROM slots.  Profile topology decides whether the
       selected board accepts a single logical image or an even/odd pair. */
    const C_CHAR *bios_path[2];
    STD_SIZE_T bios_count;
    const C_CHAR *video_path;
    vm_machine_floppy_format floppy_format;
    C_INT create_fdd;
    type_unsigned_16 create_hdd_cylinders;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
} vm_machine_config;

/* Immutable bytes resolved before session composition.  Product YAML/file
 * loading creates this bundle; repository-only unit tests may supply literal
 * bytes.  Composition never opens an asset path. */
typedef struct vm_machine_asset_bytes {
    const type_unsigned_8 *data;
    STD_SIZE_T bytes;
} vm_machine_asset_bytes;

typedef struct vm_machine_assets {
    vm_machine_asset_bytes bios[2];
    vm_machine_asset_bytes video;
    vm_machine_asset_bytes cmos_seed;
    vm_machine_asset_bytes font;
} vm_machine_assets;

typedef struct vm_machine vm_machine;

typedef struct vm_machine_reset_vector {
    type_unsigned_16 cs;
    type_unsigned_16 ip;
} vm_machine_reset_vector;

C_INT vm_machine_create(const vm_machine_config *config, vm_machine **out_session);
type_status vm_machine_create_from_assets(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_session);
C_VOID vm_machine_destroy(vm_machine *session);
type_status vm_machine_reconfigure_memory(vm_machine *session,
    STD_SIZE_T memory_bytes);
type_status vm_machine_get_speed(const vm_machine *session,
    vm_machine_speed *out_speed);
type_status vm_machine_set_speed(vm_machine *session, vm_machine_speed speed);
C_INT vm_machine_insert_fdd(vm_machine *session, const C_CHAR *path);
C_INT vm_machine_remove_fdd(vm_machine *session, const C_CHAR *path);
C_INT vm_machine_insert_hdd(vm_machine *session, const C_CHAR *path);
/* Production host-input ingress.  Events are copied into the session's
 * ordered request transport; they do not mutate guest devices synchronously. */
type_status vm_machine_submit_host_input(vm_machine *session,
    const core_machine_guest_input_event *event);
/* vm/app uses this value-only ingress; vm/machine alone translates it to
 * the Core-owned guest-input source. */
type_status vm_machine_submit_input(vm_machine *session,
    const vm_machine_input *input);
type_status vm_machine_get_reset_vector(const vm_machine *session,
    vm_machine_reset_vector *out_vector);
C_INT vm_machine_is_running(const vm_machine *session);
C_VOID vm_machine_print_machine(const vm_machine *session);
C_VOID vm_machine_print_bios(const vm_machine *session);
C_VOID vm_machine_print_status(const vm_machine *session);
type_status vm_machine_run_debugger(vm_machine *session);
type_status vm_machine_record_start(vm_machine *session, const C_CHAR *path);
type_status vm_machine_record_stop(vm_machine *session);
/* The sole copied completion route.  It transfers neither a Core object nor
 * an executor or UI handle across this boundary. */
C_VOID vm_machine_set_result_sink(vm_machine *machine,
    vm_machine_result_sink sink, C_VOID *context);

#endif
