#include "app-myob300/ob300_card.h"
#include "app-myob300/ob300_card_identity.h"
#include "app-myob300/ob300_machine_interface.h"
#include "lib/types/file.h"
#include "x86-nxvm/product/startup_interface.h"

#define MYOB300_SYSTEM_CARD_PATH "../../../nxvm-assets/hp-omnibook300/rom/system10-aba-common.bin"

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
    if (myob300_card_identity_matches(image->data, image->bytes)) return LIB_STATUS_OK;
    out->profile.release(out->profile.context);
    *out = (x86_nxvm_machine_construction){0};
    lib_c_fprintf(lib_c_stderr,
        "MyOB300 System Card does not match the supported System 1.0 ABA image.\n");
    return LIB_STATUS_INVALID_ARGUMENT;
}

int main(void)
{
    const x86_nxvm_product_machine_binding binding = {
        .name = "hp-omnibook-300",
        .cpu = X86_CORE_CPU_PROFILE_AM386SXLV,
        .fpu = X86_FPU_PROFILE_NONE,
        .initial_speed = X86_NXVM_MACHINE_SPEED_TURBO,
        .floppy_format = X86_NXVM_MACHINE_FLOPPY_FORMAT_1440K,
        .prepare = myob300_product_prepare
    };

    return x86_nxvm_product_run("MyOB300", "myob300.ini", &binding);
}
