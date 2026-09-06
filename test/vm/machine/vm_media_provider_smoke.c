#include "type.h"

#include "core/machine/media_interface.h"
#include "vm/machine/fdd_private.h"
#include "vm/machine/hdd_private.h"

static const C_CHAR vm_media_fdd_path[] = "vm-media-overlay-fdd.img";
static const C_CHAR vm_media_hdd_path[] = "vm-media-overlay-hdd.img";

static C_INT vm_media_write_file(const C_CHAR *path, const C_VOID *bytes,
    STD_SIZE_T count)
{
    STD_FILE *file = STD_FOPEN(path, "wb");
    C_INT failed = file == STD_NULL || (count != 0u &&
        STD_FWRITE(bytes, 1u, count, file) != count);

    if (file != STD_NULL && STD_FCLOSE(file) != 0) failed = TYPE_TRUE;
    return failed;
}

static C_INT vm_media_read_first(const C_CHAR *path, type_unsigned_8 expected)
{
    STD_FILE *file = STD_FOPEN(path, "rb");
    type_unsigned_8 value = 0u;
    C_INT failed = file == STD_NULL || STD_FREAD(&value, 1u, 1u, file) != 1u ||
        value != expected;

    if (file != STD_NULL && STD_FCLOSE(file) != 0) failed = TYPE_TRUE;
    return failed;
}

C_INT main(C_VOID)
{
    static type_unsigned_8 fdd_bytes[80u * 2u * 18u * 512u];
    static type_unsigned_8 hdd_bytes[1024u];
    t_fdd fdd;
    t_hdd hdd;
    core_machine_media_registry *registry = STD_NULL;
    core_machine_media_result result;
    type_unsigned_8 value = 0x5au;
    C_INT failed = TYPE_FALSE;

    fdd_bytes[0] = 0x11u;
    hdd_bytes[0] = 0x22u;
    if (vm_media_write_file(vm_media_fdd_path, fdd_bytes, sizeof(fdd_bytes)) ||
        vm_media_write_file(vm_media_hdd_path, hdd_bytes, sizeof(hdd_bytes))) failed = TYPE_TRUE;
    vm_machine_fdd_initialize(&fdd);
    vm_machine_hdd_initialize(&hdd);
    if (!failed && (vm_machine_fdd_insert_for(&fdd, vm_media_fdd_path) != TYPE_FALSE ||
        vm_machine_hdd_insert(&hdd, vm_media_hdd_path) != TYPE_FALSE ||
        core_machine_media_registry_create(&registry) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, &fdd,
            vm_machine_fdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 2u, &hdd,
            vm_machine_hdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_freeze(registry) != TYPE_STATUS_OK ||
        core_machine_media_write_bytes(registry, 1u, 0u, &value, 1u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_write_bytes(registry, 2u, 0u, &value, 1u,
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        vm_machine_fdd_remove_for(&fdd, vm_media_fdd_path) != TYPE_FALSE ||
        vm_machine_hdd_remove(&hdd, vm_media_hdd_path) != TYPE_FALSE ||
        vm_media_read_first(vm_media_fdd_path, 0x11u) ||
        vm_media_read_first(vm_media_hdd_path, 0x22u))) failed = TYPE_TRUE;
    core_machine_media_registry_destroy(registry);
    vm_machine_fdd_finalize(&fdd);
    vm_machine_hdd_finalize(&hdd);
    (C_VOID)STD_REMOVE(vm_media_fdd_path);
    (C_VOID)STD_REMOVE(vm_media_hdd_path);
    return failed;
}
