#ifndef TEST_VM_SESSION_FIXTURE_H
#define TEST_VM_SESSION_FIXTURE_H

#include "vm/composition/session/session_interface.h"
#include "core/machine/block_provider.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/machine/cmos.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdc.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/pc_at_profile.h"

C_INT vm_session_fixture_is_active(const vm_session *session);
vm_session *vm_session_fixture_allocate(C_VOID);
C_VOID vm_session_fixture_free(vm_session *session);
C_VOID vm_session_fixture_storage_initialize(vm_session *session);
C_VOID vm_session_fixture_storage_finalize(vm_session *session);
core_machine *vm_session_fixture_machine(const vm_session *session);
t_cmos *vm_session_fixture_cmos(const vm_session *session);
t_fdd *vm_session_fixture_fdd(const vm_session *session);
t_fdc *vm_session_fixture_fdc(const vm_session *session);
t_hdd *vm_session_fixture_hdd(const vm_session *session);
vm_machine_hdc *vm_session_fixture_hdc(const vm_session *session);
t_debug *vm_session_fixture_debug(const vm_session *session);
core_machine_block_provider_slot *vm_session_fixture_block_provider(const vm_session *session);
vm_session_control_state *vm_session_fixture_control(const vm_session *session);
vm_platform_request_transport *vm_session_fixture_request_transport(const vm_session *session);
vm_platform_mouse_transport *vm_session_fixture_mouse_transport(const vm_session *session);
vm_platform_presentation_mailbox *vm_session_fixture_presentation_mailbox(const vm_session *session);
vm_platform_run_context *vm_session_fixture_platform_run_context(const vm_session *session);
vm_platform_run_handle *vm_session_fixture_platform_run_handle(const vm_session *session);
t_bios *vm_session_fixture_default_bios(const vm_session *session);
const vm_profile_default_pc_at_descriptor *vm_session_fixture_profile(
    const vm_session *session);

#endif
