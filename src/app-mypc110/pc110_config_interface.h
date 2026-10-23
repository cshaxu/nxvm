#ifndef MYPC110_CONFIG_INTERFACE_H
#define MYPC110_CONFIG_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/chips/vl82c420/vl82c420_interface.h"
#include "x86-core/machine/port_interface.h"

typedef struct mypc110_config_routes {
    x86_vl82c420 *chip;
} mypc110_config_routes;

lib_status mypc110_config_routes_initialize(mypc110_config_routes *routes,
    x86_vl82c420 *chip);
lib_status mypc110_config_install_routes(x86_core_machine *machine,
    mypc110_config_routes *routes);

#endif
