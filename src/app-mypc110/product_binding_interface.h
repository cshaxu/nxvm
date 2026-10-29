#ifndef MYPC110_PRODUCT_BINDING_INTERFACE_H
#define MYPC110_PRODUCT_BINDING_INTERFACE_H

#include "x86-nxvm/product/factory_interface.h"
#include "app-mypc110/product_vdisplay.h"

extern const x86_nxvm_product_machine_binding vm_app_machine;
lib_status mypc110_product_initialize_vdisplay(mypc110_vdisplay *display);

#endif
