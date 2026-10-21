#include "myraidenii/product/composition_interface.h"
#include "myraidenii/product/config_interface.h"

int main(int argc, char **argv)
{
    myraidenii_product_config config;

    if (!myraidenii_product_config_load((lib_i32)argc, argv, &config)) return 2;
    return (int)myraidenii_product_run(&config);
}
