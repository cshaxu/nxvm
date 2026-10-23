#include "app-myraidenii/product/composition_interface.h"
#include "app-myraidenii/product/config_interface.h"
#include "lib/types/file.h"

int main(int argc, char **argv)
{
    myraidenii_product_config config;
    lib_status status = myraidenii_product_config_load((lib_i32)argc, argv, &config);

    if (status != LIB_STATUS_OK) {
        if (status == LIB_STATUS_INVALID_ARGUMENT)
            lib_c_fprintf(lib_c_stderr, "Invalid arguments or myraidenii.ini configuration.\n");
        else
            lib_c_fprintf(lib_c_stderr, "Cannot read myraidenii.ini (status %d).\n", (int)status);
        return 2;
    }
    return (int)myraidenii_product_run(&config);
}
