#include "type.h"

#include "core/machine/media_interface.h"
#include "vm/machine/fdd_private.h"
#include "vm/machine/hdd_private.h"

static const C_CHAR vm_media_direct_fdd_path[] = "vm_media_direct_fdd.img";
static const C_CHAR vm_media_direct_hdd_path[] = "vm_media_direct_hdd.img";
static type_unsigned_8 vm_media_direct_fdd_bytes[80u * 2u * 18u * 512u];

static C_INT vm_media_direct_write(const C_CHAR *path, const C_VOID *bytes,
    STD_SIZE_T byte_count)
{
    STD_FILE *file = STD_FOPEN(path, "wb");
    C_INT failed = file == STD_NULL ||
        (byte_count != 0u && STD_FWRITE(bytes, 1u, byte_count, file) != byte_count);

    if (file != STD_NULL && STD_FCLOSE(file) != 0) failed = 1;
    return failed;
}

static C_INT vm_media_direct_read_first(const C_CHAR *path,
    type_unsigned_8 expected)
{
    STD_FILE *file = STD_FOPEN(path, "rb");
    type_unsigned_8 value = 0u;
    C_INT failed = file == STD_NULL || STD_FREAD(&value, 1u, 1u, file) != 1u ||
        value != expected;

    if (file != STD_NULL && STD_FCLOSE(file) != 0) failed = 1;
    return failed;
}

C_INT main(C_VOID)
{
    t_fdd fdd;
    t_hdd hdd;
    type_unsigned_8 hdd_bytes[512] = {0x5au};
    type_unsigned_8 direct_value = 0x3cu;
    type_unsigned_8 byte = 0u;
    C_INT failed = 0;

    vm_media_direct_fdd_bytes[0u] = 0xa5u;
    if (vm_media_direct_write(vm_media_direct_fdd_path, vm_media_direct_fdd_bytes,
            sizeof(vm_media_direct_fdd_bytes)) ||
        vm_media_direct_write(vm_media_direct_hdd_path, hdd_bytes, sizeof(hdd_bytes))) {
        return 1;
    }
    vm_machine_fdd_initialize(&fdd);
    vm_machine_hdd_initialize(&hdd);
    if (vm_machine_fdd_insert_readonly_for(&fdd, vm_media_direct_fdd_path) != TYPE_FALSE ||
        !fdd.connect.flagReadOnly || vm_machine_fdd_read_byte(&fdd, 0u, 0u, 1u, 0u,
            &byte) != TYPE_FALSE || byte != 0xa5u ||
        vm_machine_fdd_write_byte(&fdd, 0u, 0u, 1u, 0u, 0u) != TYPE_TRUE ||
        vm_machine_hdd_insert_readonly(&hdd, vm_media_direct_hdd_path) != TYPE_FALSE ||
        !hdd.connect.flagReadOnly || vm_machine_hdd_media_provider()->write_bytes(&hdd,
            0u, &byte, 1u) != CORE_MACHINE_MEDIA_RESULT_READ_ONLY ||
        vm_machine_hdd_media_provider()->read_bytes(&hdd, 0u, &byte, 1u) !=
            CORE_MACHINE_MEDIA_RESULT_OK || byte != 0x5au) {
        failed = 1;
    }
    if (vm_machine_fdd_remove_for(&fdd, STD_NULL) != TYPE_FALSE ||
        vm_machine_hdd_remove(&hdd, STD_NULL) != TYPE_FALSE) failed = 1;
    if (!failed && (vm_machine_fdd_insert_direct_for(&fdd,
            vm_media_direct_fdd_path) != TYPE_FALSE ||
        vm_machine_fdd_write_byte(&fdd, 0u, 0u, 1u, 0u, direct_value) != TYPE_FALSE ||
        vm_machine_hdd_insert_direct(&hdd, vm_media_direct_hdd_path) != TYPE_FALSE ||
        vm_machine_hdd_media_provider()->write_bytes(&hdd, 0u, &direct_value,
            1u) != CORE_MACHINE_MEDIA_RESULT_OK ||
        vm_machine_fdd_remove_for(&fdd, STD_NULL) != TYPE_FALSE ||
        vm_machine_hdd_remove(&hdd, STD_NULL) != TYPE_FALSE ||
        vm_media_direct_read_first(vm_media_direct_fdd_path, direct_value) ||
        vm_media_direct_read_first(vm_media_direct_hdd_path, direct_value))) failed = 1;
    vm_machine_fdd_finalize(&fdd);
    vm_machine_hdd_finalize(&hdd);
    (C_VOID)STD_REMOVE(vm_media_direct_fdd_path);
    (C_VOID)STD_REMOVE(vm_media_direct_hdd_path);
    if (failed) return 1;
    STD_PRINTF("M5:T524:S10:MEDIA-DIRECT-READONLY:OK\n");
    return 0;
}
