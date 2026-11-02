#ifndef MYPC110_PRODUCT_BINDING_INTERFACE_H
#define MYPC110_PRODUCT_BINDING_INTERFACE_H

#include "x86-nxvm/product/factory_interface.h"
#include "app-mypc110/product_context.h"

extern const x86_nxvm_product_machine_binding vm_app_machine;
lib_status mypc110_product_initialize(mypc110_product_context *context);

#endif
