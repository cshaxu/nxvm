#include "app-myob300/ob300_card.h"
#include "app-myob300/ob300_card_identity.h"
#include "app-myob300/ob300_machine_interface.h"
#include "lib/storage/file_interface.h"
#include "lib/types/file.h"
#include "x86-nxvm/product/startup_interface.h"

#define MYOB300_SYSTEM_CARD_PATH "../../../nxvm-assets/hp-omnibook300/rom/system10-aba-common.bin"
#define MYOB300_CMOS_FILE "myob300.cmos"

static lib_status myob300_product_load_cmos(x86_nxvm_machine_construction *out)
{
    lib_u8 path[EMULATOR_MACHINE_PATH_CAPACITY];
    lib_storage_file_writer *file = LIB_NULL;
    void *image = LIB_NULL;
    lib_size bytes = 0u;
    lib_status status = x86_nxvm_product_ini_executable_path(MYOB300_CMOS_FILE,
        path, sizeof(path));

    /* An empty file represents a machine with no previous configuration. */
    if (status == LIB_STATUS_OK)
        status = lib_storage_file_writer_open((const char *)path,
            LIB_STORAGE_FILE_WRITER_APPEND, &file);
    if (status == LIB_STATUS_OK) status = lib_storage_file_writer_close(file);
    if (status == LIB_STATUS_OK)
        status = lib_storage_file_read_owned((const char *)path,
            sizeof(out->cmos_seed), &image, &bytes);
    if (status == LIB_STATUS_OK && bytes != 0u) {
        if (bytes != sizeof(out->cmos_seed)) status = LIB_STATUS_INVALID_ARGUMENT;
        else {
            lib_memory_copy(out->cmos_seed, image, bytes);
            out->cmos_seed_bytes = bytes;
            out->cmos_seed_present = LIB_TRUE;
            lib_memory_copy(out->rtc_seed, image, sizeof(out->rtc_seed));
            out->rtc_seed_present = LIB_TRUE;
        }
    }
    lib_release(image);
    return status;
}

static lib_status myob300_product_save_cmos(void *context, x86_nxvm_machine *machine)
{
    lib_u8 image[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    lib_u8 path[EMULATOR_MACHINE_PATH_CAPACITY];
    lib_storage_file_writer *file = LIB_NULL;
    lib_status status = x86_nxvm_machine_capture_cmos(machine, image, sizeof(image));

    (void)context;
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_capture_rtc(machine, image, X86_NXVM_MACHINE_RTC_SEED_BYTES);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_product_ini_executable_path(MYOB300_CMOS_FILE, path, sizeof(path));
    if (status == LIB_STATUS_OK)
        status = lib_storage_file_writer_open((const char *)path,
            LIB_STORAGE_FILE_WRITER_TRUNCATE, &file);
    if (status == LIB_STATUS_OK) status = lib_storage_file_writer_write(file, image, sizeof(image));
    if (file != LIB_NULL) {
        const lib_status close_status = lib_storage_file_writer_close(file);
        if (status == LIB_STATUS_OK) status = close_status;
    }
    if (status != LIB_STATUS_OK)
        lib_c_fprintf(lib_c_stderr, "MyOB300 could not save %s.\n", MYOB300_CMOS_FILE);
    return status;
}

static lib_status myob300_product_prepare(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *out)
{
    lib_u8 path[EMULATOR_MACHINE_PATH_CAPACITY];
    lib_status status;

    (void)assets;
    status = x86_nxvm_product_ini_executable_path(MYOB300_SYSTEM_CARD_PATH,
        path, sizeof(path));
    if (status != LIB_STATUS_OK) return status;
    status = myob300_machine_prepare(config, (const char *)path, out);
    if (status != LIB_STATUS_OK) {
        lib_c_fprintf(lib_c_stderr,
            "MyOB300 could not prepare the machine. Check the INI and external System Card.\n");
        return status;
    }
    const myob300_card_image *image = out->firmware_context;
    if (myob300_card_identity_matches(image->data, image->bytes)) {
        status = myob300_product_load_cmos(out);
        if (status == LIB_STATUS_OK) return status;
        lib_c_fprintf(lib_c_stderr, "MyOB300 could not load %s.\n", MYOB300_CMOS_FILE);
    } else {
        status = LIB_STATUS_INVALID_ARGUMENT;
        lib_c_fprintf(lib_c_stderr,
            "MyOB300 System Card does not match the supported System 1.0 ABA image.\n");
    }
    out->profile.release(out->profile.context);
    *out = (x86_nxvm_machine_construction){0};
    return status;
}

int main(void)
{
    const x86_nxvm_product_machine_binding binding = {
        .name = "hp-omnibook-300",
        .cpu = X86_CORE_CPU_PROFILE_AM386SXLV,
        .fpu = X86_FPU_PROFILE_NONE,
        .initial_speed = X86_NXVM_MACHINE_SPEED_TURBO,
        .floppy_format = X86_NXVM_MACHINE_FLOPPY_FORMAT_1440K,
        .prepare = myob300_product_prepare,
        .save_configuration = myob300_product_save_cmos
    };

    return x86_nxvm_product_run("MyOB300", "myob300.ini", &binding);
}
