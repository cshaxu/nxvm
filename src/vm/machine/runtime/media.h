#ifndef VM_MACHINE_MEDIA_H
#define VM_MACHINE_MEDIA_H


#include "type.h"
struct vm_machine;
#define VM_MACHINE_MEDIA_FDD_ID 1u
#define VM_MACHINE_MEDIA_HDD_ID 2u
#define VM_MACHINE_MEDIA_FDD_SECONDARY_ID 3u
#define VM_MACHINE_MEDIA_HDD_SECONDARY_ID 4u

type_status vm_machine_bind_media(struct vm_machine *machine);

#endif
