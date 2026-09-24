#include "product/composition.h"

int main(int argc, char **argv)
{
    app_startup_config config;

    if (!app_config_parse((lib_i32)argc, argv, &config)) return 2;
    return (int)app_composition_run(&config);
}
