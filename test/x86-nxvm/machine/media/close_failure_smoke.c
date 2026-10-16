#include "lib/types/types_interface.h"

#include "x86-nxvm/machine/media/fdd.h"
#include "x86-nxvm/machine/media/hdd.h"

static lib_bool vm_media_close_failure_pending;
static lib_bool vm_media_allocation_failure_pending;
static lib_bool vm_media_write_failure_pending;
static lib_bool vm_media_read_failure_pending;
static lib_u32 vm_media_live_allocations;
static lib_u32 vm_media_write_calls;

static void *vm_media_failure_allocate(lib_size count, lib_size size)
{
    void *memory;
    if (vm_media_allocation_failure_pending) return LIB_NULL;
    memory = lib_allocate_zero(count, size);
    if (memory != LIB_NULL) ++vm_media_live_allocations;
    return memory;
}

static void vm_media_failure_release(void *memory)
{
    if (memory != LIB_NULL) --vm_media_live_allocations;
    lib_release(memory);
}

static lib_status vm_media_failure_write(lib_storage_medium *medium,
    lib_size offset, const void *buffer, lib_size count)
{
    ++vm_media_write_calls;
    if (vm_media_write_failure_pending) return LIB_STATUS_IO_ERROR;
    return lib_storage_medium_write_at(medium, offset, buffer, count);
}

static lib_status vm_media_failure_read(const lib_storage_medium *medium,
    lib_size offset, void *buffer, lib_size count)
{
    if (vm_media_read_failure_pending) {
        /* A provider may modify its borrowed buffer before reporting failure. */
        if (count != 0u) ((lib_u8 *)buffer)[0] = 0xedu;
        return LIB_STATUS_IO_ERROR;
    }
    return lib_storage_medium_read_at(medium, offset, buffer, count);
}

static lib_status vm_media_failure_fill(lib_storage_medium *medium,
    lib_size offset, lib_size count, lib_u8 fill)
{
    ++vm_media_write_calls;
    if (vm_media_write_failure_pending) return LIB_STATUS_IO_ERROR;
    return lib_storage_medium_fill_at(medium, offset, count, fill);
}

/* The injected result occurs only after Storage has consumed the lease. */
static lib_status vm_media_close_failure_destroy(lib_storage_medium **medium)
{
    lib_status status = lib_storage_medium_destroy(medium);

    if (status != LIB_STATUS_OK || !vm_media_close_failure_pending) return status;
    vm_media_close_failure_pending = LIB_FALSE;
    return LIB_STATUS_IO_ERROR;
}

/* Include after Types so inline facade declarations are not renamed. */
#define lib_allocate_zero vm_media_failure_allocate
#define lib_release vm_media_failure_release
#define lib_storage_medium_write_at vm_media_failure_write
#define lib_storage_medium_read_at vm_media_failure_read
#define lib_storage_medium_fill_at vm_media_failure_fill
#define lib_storage_medium_destroy vm_media_close_failure_destroy
#include "x86-nxvm/machine/media/fdd.c"
#include "x86-nxvm/machine/media/hdd.c"
#undef lib_allocate_zero
#undef lib_release
#undef lib_storage_medium_write_at
#undef lib_storage_medium_read_at
#undef lib_storage_medium_fill_at
#undef lib_storage_medium_destroy

static lib_bool vm_media_record_failures(t_hdd *hdd)
{
    const x86_nxvm_board_media_provider *provider = x86_nxvm_machine_hdd_media_provider();
    x86_nxvm_board_media_record record = {.data = {0x5au},
        .ecc = {1u, 2u, 3u, 4u}, .ecc_present = LIB_TRUE};
    x86_nxvm_board_media_record observed = {0};
    lib_u32 live = vm_media_live_allocations;
    lib_u32 writes = vm_media_write_calls;

    vm_media_allocation_failure_pending = LIB_TRUE;
    if (provider->write_record(hdd, 0u, &record) != X86_NXVM_BOARD_MEDIA_RESULT_TRANSIENT ||
        vm_media_live_allocations != live || vm_media_write_calls != writes)
        return LIB_TRUE;
    vm_media_allocation_failure_pending = LIB_FALSE;
    if (provider->write_record(hdd, 0u, &record) != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        vm_media_live_allocations != live + 1u) return LIB_TRUE;
    record.data[0] = 0x42u;
    record.ecc[0] = 0xabu;
    vm_media_write_failure_pending = LIB_TRUE;
    if (provider->write_record(hdd, 0u, &record) != X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT ||
        vm_media_live_allocations != live + 1u) return LIB_TRUE;
    vm_media_write_failure_pending = LIB_FALSE;
    if (provider->read_record(hdd, 0u, &observed) != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        observed.data[0] != 0x5au || !observed.ecc_present || observed.ecc[0] != 1u)
        return LIB_TRUE;
    /* Ordinary writes with no ECC must also preserve old facts on failure. */
    record.ecc_present = LIB_FALSE;
    vm_media_write_failure_pending = LIB_TRUE;
    if (provider->write_record(hdd, 0u, &record) != X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT ||
        vm_media_live_allocations != live + 1u) return LIB_TRUE;
    vm_media_write_failure_pending = LIB_FALSE;
    if (provider->read_record(hdd, 0u, &observed) != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        observed.data[0] != 0x5au || !observed.ecc_present || observed.ecc[0] != 1u)
        return LIB_TRUE;
    observed.data[0] = 0x55u;
    observed.ecc[0] = 0x66u;
    vm_media_read_failure_pending = LIB_TRUE;
    if (provider->read_record(hdd, 0u, &observed) != X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT ||
        observed.data[0] != 0x55u || observed.ecc[0] != 0x66u || !observed.ecc_present)
        return LIB_TRUE;
    vm_media_read_failure_pending = LIB_FALSE;
    {
        x86_nxvm_board_media_track track = {.sector_count = 1u};
        x86_nxvm_machine_hdd_track *installed;
        const lib_u32 before = vm_media_live_allocations;
        const lib_u32 before_writes = vm_media_write_calls;

        vm_media_allocation_failure_pending = LIB_TRUE;
        if (provider->format_track(hdd, &track) != X86_NXVM_BOARD_MEDIA_RESULT_TRANSIENT ||
            vm_media_live_allocations != before || hdd->connect.tracks != LIB_NULL ||
            vm_media_write_calls != before_writes) return LIB_TRUE;
        vm_media_allocation_failure_pending = LIB_FALSE;
        if (provider->format_track(hdd, &track) != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
            vm_media_live_allocations != before) return LIB_TRUE;
        /* Format consumes the old tag; install a new record before fill failure. */
        record.data[0] = 0x5au;
        record.ecc[0] = 1u;
        record.ecc_present = LIB_TRUE;
        if (provider->write_record(hdd, 0u, &record) != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
            vm_media_live_allocations != before + 1u) return LIB_TRUE;
        installed = hdd->connect.tracks;
        track.bad[0] = LIB_TRUE;
        track.fill_payload = LIB_TRUE;
        vm_media_write_failure_pending = LIB_TRUE;
        if (provider->format_track(hdd, &track) != X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT ||
            vm_media_live_allocations != before + 1u || hdd->connect.tracks != installed ||
            hdd->connect.tracks->format.bad[0]) return LIB_TRUE;
        vm_media_write_failure_pending = LIB_FALSE;
        if (provider->read_record(hdd, 0u, &observed) != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
            observed.data[0] != 0x5au || !observed.ecc_present || observed.ecc[0] != 1u)
            return LIB_TRUE;
    }
    return LIB_FALSE;
}

int main(void)
{
    static const x86_nxvm_board_media_geometry fdd_geometry = {
        1u, 512u, 1u, 1u, 1u
    };
    static const lib_u8 fdd_bytes[512] = { 0xa5u };
    static const lib_u8 hdd_bytes[512] = { 0x5au };
    static const lib_u8 replacement_fdd_bytes[512] = { 0x3cu };
    static const lib_u8 replacement_hdd_bytes[512] = { 0xc3u };
    t_fdd fdd;
    t_hdd hdd;
    lib_u8 byte;

    if (x86_nxvm_machine_fdd_initialize_with_geometry(&fdd, &fdd_geometry) != LIB_STATUS_OK)
        return 1;
    x86_nxvm_machine_hdd_initialize(&hdd);
    if (x86_nxvm_machine_fdd_replace_bytes(&fdd, fdd_bytes, sizeof(fdd_bytes)) != LIB_STATUS_OK ||
        x86_nxvm_machine_hdd_replace_bytes(&hdd, hdd_bytes, sizeof(hdd_bytes)) != LIB_STATUS_OK)
        return 1;
    if (vm_media_record_failures(&hdd)) return 1;

    vm_media_close_failure_pending = LIB_TRUE;
    if (x86_nxvm_machine_fdd_replace_bytes(&fdd, replacement_fdd_bytes,
            sizeof(replacement_fdd_bytes)) != LIB_STATUS_IO_ERROR ||
        !x86_nxvm_machine_fdd_has_media(&fdd) ||
        x86_nxvm_machine_fdd_read_byte(&fdd, 0u, 0u, 1u, 0u, &byte) != LIB_STATUS_OK ||
        byte != replacement_fdd_bytes[0u]) return 1;
    vm_media_close_failure_pending = LIB_TRUE;
    if (x86_nxvm_machine_hdd_replace_bytes(&hdd, replacement_hdd_bytes,
            sizeof(replacement_hdd_bytes)) != LIB_STATUS_IO_ERROR ||
        !x86_nxvm_machine_hdd_has_media(&hdd) ||
        x86_nxvm_machine_hdd_media_provider()->read_bytes(&hdd, 0u, &byte, 1u) !=
            X86_NXVM_BOARD_MEDIA_RESULT_OK || byte != replacement_hdd_bytes[0u]) return 1;
    if (hdd.connect.records != LIB_NULL || hdd.connect.tracks != LIB_NULL) return 1;
    {
        x86_nxvm_board_media_record record = {.ecc = {4u, 3u, 2u, 1u},
            .ecc_present = LIB_TRUE};
        if (x86_nxvm_machine_hdd_media_provider()->write_record(&hdd, 0u, &record) !=
                X86_NXVM_BOARD_MEDIA_RESULT_OK) return 1;
    }

    vm_media_close_failure_pending = LIB_TRUE;
    if (x86_nxvm_machine_fdd_remove_for(&fdd) != LIB_STATUS_IO_ERROR ||
        x86_nxvm_machine_fdd_has_media(&fdd) || fdd.connect.medium != LIB_NULL)
        return 1;
    vm_media_close_failure_pending = LIB_TRUE;
    if (x86_nxvm_machine_hdd_remove(&hdd) != LIB_STATUS_IO_ERROR ||
        x86_nxvm_machine_hdd_has_media(&hdd) || hdd.connect.medium != LIB_NULL ||
        hdd.connect.records != LIB_NULL)
        return 1;
    x86_nxvm_machine_fdd_finalize(&fdd);
    x86_nxvm_machine_hdd_finalize(&hdd);
    if (vm_media_live_allocations != 0u) return 1;
    return 0;
}
