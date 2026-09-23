#include "product/composition.h"

int main(int argc, char **argv)
{
    app_startup_config config;

    if (!app_config_parse(argc, argv, &config)) return 2;
    return app_composition_run(&config);
}
