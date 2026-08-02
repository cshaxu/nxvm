#ifndef NTVDM64_VM_COMPOSITION_DISPLAY_H
#define NTVDM64_VM_COMPOSITION_DISPLAY_H

struct vm_composition_live_machine;
void vm_composition_bind_display(struct vm_composition_live_machine *machine);
void vm_composition_publish_display(struct vm_composition_live_machine *machine,
    int force);

#endif
