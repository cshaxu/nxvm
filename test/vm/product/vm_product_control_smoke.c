#include "type.h"
#include "vm/product/control.h"

int main(void)
{
    vm_product_control *control = STD_NULL;
    vm_product_control_fact fact = {0};
    vm_product_control_fact received = {0};

    if (vm_product_control_create(&control) != TYPE_STATUS_OK) return 1;
    fact.kind = VM_PRODUCT_CONTROL_FACT_LIFECYCLE;
    fact.value.lifecycle = VM_SESSION_RUNNING;
    if (vm_product_control_publish(control, &fact) != TYPE_STATUS_OK ||
        vm_product_control_take(control, &received, 0u) != TYPE_STATUS_OK ||
        received.kind != VM_PRODUCT_CONTROL_FACT_LIFECYCLE ||
        received.value.lifecycle != VM_SESSION_RUNNING ||
        STD_STRCMP(vm_product_control_note_lifecycle(control,
            received.value.lifecycle), "started") != 0 ||
        STD_STRCMP(vm_product_control_note_lifecycle(control,
            VM_SESSION_PAUSED), "paused") != 0 ||
        STD_STRCMP(vm_product_control_note_lifecycle(control,
            VM_SESSION_RUNNING), "resumed") != 0) {
        vm_product_control_destroy(control);
        return 1;
    }
    vm_product_control_close(control);
    if (vm_product_control_publish(control, &fact) == TYPE_STATUS_OK) {
        vm_product_control_destroy(control);
        return 1;
    }
    vm_product_control_destroy(control);
    return 0;
}
