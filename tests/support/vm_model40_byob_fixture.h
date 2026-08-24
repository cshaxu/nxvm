#ifndef TESTS_SUPPORT_VM_MODEL40_BYOB_FIXTURE_H
#define TESTS_SUPPORT_VM_MODEL40_BYOB_FIXTURE_H

#include "type.h"
#include "vm/composition/session/session_interface.h"

static inline C_INT vm_model40_fixture_write_bytes(const C_CHAR *path,
    const type_unsigned_8 *bytes)
{
    STD_FILE *file = STD_FOPEN(path, "wb");

    return file != STD_NULL && STD_FWRITE(bytes, 1u,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES, file) ==
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES && STD_FCLOSE(file) == 0;
}

static inline type_status vm_model40_fixture_create_bytes(const C_CHAR *even_path,
    const type_unsigned_8 *even_bytes, const C_CHAR *even_sha256,
    const C_CHAR *odd_path, const type_unsigned_8 *odd_bytes,
    const C_CHAR *odd_sha256, vm_session **out_session)
{
    vm_session_config config = {0};

    if (!vm_model40_fixture_write_bytes(even_path, even_bytes) ||
        !vm_model40_fixture_write_bytes(odd_path, odd_bytes)) return TYPE_STATUS_FAULT;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.model40_firmware = (vm_profile_model40_byob_manifest) {
        even_path, even_sha256, odd_path, odd_sha256,
        "project-owned synthetic test input" };
    return (type_status)vm_session_create(&config, out_session);
}

static inline type_status vm_model40_fixture_create(const C_CHAR *even_path,
    const C_CHAR *odd_path, vm_session **out_session)
{
    static const C_CHAR even_sha256[] =
        "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe";
    static const C_CHAR odd_sha256[] =
        "111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f";
    type_unsigned_8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
    type_unsigned_8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];

    STD_MEMSET(odd_bytes, 1, sizeof(odd_bytes));
    return vm_model40_fixture_create_bytes(even_path, even_bytes, even_sha256,
        odd_path, odd_bytes, odd_sha256, out_session);
}
static inline C_VOID vm_model40_fixture_remove(const C_CHAR *even_path,
    const C_CHAR *odd_path)
{
    (C_VOID)STD_REMOVE(even_path);
    (C_VOID)STD_REMOVE(odd_path);
}

#endif
