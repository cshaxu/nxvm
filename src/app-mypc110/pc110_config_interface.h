#ifndef MYPC110_CONFIG_INTERFACE_H
#define MYPC110_CONFIG_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/chips/vl82c420/vl82c420_interface.h"
#include "x86-core/machine/port_interface.h"

typedef struct mypc110_firmware mypc110_firmware;

lib_status mypc110_config_initialize_cold_image(x86_vl82c420 *chip);
/* Install after this firmware's memory routes, on the same Core. A failed
 * installation leaves the borrowed configuration chip unchanged. */
lib_status mypc110_config_install_routes(x86_core_machine *machine,
    mypc110_firmware *firmware, x86_vl82c420 *chip);

#endif
