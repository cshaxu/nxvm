#ifndef VM_APP_COMPOSITION_H
#define VM_APP_COMPOSITION_H

#include "ibmpc/product/composition_interface.h"

lib_status vm_app_information_read(const vm_app *app, vm_app_information *out_info);
lib_status vm_app_speed_read(const vm_app *app, vm_app_speed *out_speed);
lib_status vm_app_speed_write(vm_app *app, vm_app_speed speed);

#endif
