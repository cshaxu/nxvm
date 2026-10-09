#ifndef PRODUCT_SURFACE_COMPOSITION_INTERFACE_H
#define PRODUCT_SURFACE_COMPOSITION_INTERFACE_H
#include "lib/types/types_interface.h"


#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"
#include "product/surface/machine_interface.h"
typedef struct product_surface product_surface;

lib_status product_surface_create(const app_composed_machine *machine,
    product_surface **out_app);
lib_status product_surface_destroy(product_surface *app);
common_session *product_surface_session(const product_surface *app);
common_machine *product_surface_common_machine(const product_surface *app);
common_ui *product_surface_ui(const product_surface *app);
lib_status product_surface_compose_machine(product_surface *app);
lib_status product_surface_compose_control(product_surface *app,
    const common_session_options *options);
lib_status product_surface_compose_ui(product_surface *app, const common_ui_options *options);
lib_status product_surface_information_read(const product_surface *app,
    product_surface_information *out_info);
lib_status product_surface_speed_read(const product_surface *app, product_surface_speed *out_speed);
lib_status product_surface_speed_write(product_surface *app, product_surface_speed speed);

#endif
