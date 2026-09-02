#ifndef VM_SESSION_MEDIA_H
#define VM_SESSION_MEDIA_H


#include "type.h"
struct vm_session;
#define VM_SESSION_MEDIA_FDD_ID 1u
#define VM_SESSION_MEDIA_HDD_ID 2u
#define VM_SESSION_MEDIA_FDD_SECONDARY_ID 3u
#define VM_SESSION_MEDIA_HDD_SECONDARY_ID 4u

type_status vm_session_bind_media(struct vm_session *machine);

#endif
