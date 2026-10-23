#ifndef MYPC110_CONFIG_INTERFACE_H
#define MYPC110_CONFIG_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/chips/vl82c420/vl82c420_interface.h"
#include "x86-core/machine/port_interface.h"

/* Installs the proven PC110 SCAMP and EC/ED index/data windows. The caller
 * retains chip ownership until it removes routes or destroys the Core machine.
 * Block2 is intentionally absent until its guest unlock-read route is proven. */
lib_status mypc110_config_install_routes(x86_core_machine *machine,
    x86_vl82c420 *chip);

#endif
