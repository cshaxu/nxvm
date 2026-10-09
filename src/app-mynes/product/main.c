#include "product/composition.h"
#include "lib/types/file.h"

#define MYNES_PRODUCT_BANNER "MyNES [0.0.0044]\n================\n\n"

int main(int argc, char **argv)
{
    app_startup_config config;

    if (!app_config_parse((lib_i32)argc, argv, &config)) return 2;
    lib_c_printf("%s", MYNES_PRODUCT_BANNER);
    return (int)app_composition_run(&config);
}
