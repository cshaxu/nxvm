#ifndef MYRAIDENII_PRODUCT_COMPOSITION_INTERFACE_H
#define MYRAIDENII_PRODUCT_COMPOSITION_INTERFACE_H

#include "lib/types/types_interface.h"
#include "app-myraidenii/product/config_interface.h"

/* Product constructs the one Board/Machine aggregate, then transfers its
 * public driver to Emulator Product for the sole Session/UI/execution route. */
lib_i32 myraidenii_product_run(const myraidenii_product_config *config);

#endif
