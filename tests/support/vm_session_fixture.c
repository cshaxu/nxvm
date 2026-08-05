#include "tests/support/vm_session_fixture.h"

#include "vm/composition/session/session_private.h"

C_INT vm_session_fixture_is_active(const vm_session *session)
{ return session != STD_NULL && session->active; }
vm_session *vm_session_fixture_allocate(C_VOID)
{ return (vm_session *)STD_CALLOC(1u, sizeof(vm_session)); }
C_VOID vm_session_fixture_free(vm_session *session)
{ STD_FREE(session); }
C_VOID vm_session_fixture_storage_initialize(vm_session *session)
{ vm_session_storage_initialize(session); }
C_VOID vm_session_fixture_storage_finalize(vm_session *session)
{ vm_session_storage_finalize(session); }
core_machine *vm_session_fixture_machine(const vm_session *session)
{ return session != STD_NULL ? session->core_machine : STD_NULL; }
t_cmos *vm_session_fixture_cmos(const vm_session *session)
{ return session != STD_NULL ? (t_cmos *)&session->cmos : STD_NULL; }
t_fdd *vm_session_fixture_fdd(const vm_session *session)
{ return session != STD_NULL ? (t_fdd *)&session->fdd : STD_NULL; }
t_fdc *vm_session_fixture_fdc(const vm_session *session)
{ return session != STD_NULL ? (t_fdc *)&session->fdc : STD_NULL; }
t_hdd *vm_session_fixture_hdd(const vm_session *session)
{ return session != STD_NULL ? (t_hdd *)&session->hdd : STD_NULL; }
vm_machine_hdc *vm_session_fixture_hdc(const vm_session *session)
{ return session != STD_NULL ? (vm_machine_hdc *)&session->hdc : STD_NULL; }
t_debug *vm_session_fixture_debug(const vm_session *session)
{ return session != STD_NULL ? (t_debug *)&session->debug : STD_NULL; }
core_machine_block_provider_slot *vm_session_fixture_block_provider(const vm_session *session)
{ return session != STD_NULL ? (core_machine_block_provider_slot *)&session->block_provider : STD_NULL; }
vm_session_control_state *vm_session_fixture_control(const vm_session *session)
{ return session != STD_NULL ? (vm_session_control_state *)&session->control : STD_NULL; }
vm_platform_request_transport *vm_session_fixture_request_transport(const vm_session *session)
{ return session != STD_NULL ? (vm_platform_request_transport *)&session->request_transport : STD_NULL; }
vm_platform_mouse_transport *vm_session_fixture_mouse_transport(const vm_session *session)
{ return session != STD_NULL ? (vm_platform_mouse_transport *)&session->mouse_transport : STD_NULL; }
vm_platform_presentation_mailbox *vm_session_fixture_presentation_mailbox(const vm_session *session)
{ return session != STD_NULL ? (vm_platform_presentation_mailbox *)&session->presentation_mailbox : STD_NULL; }
vm_platform_run_context *vm_session_fixture_platform_run_context(const vm_session *session)
{ return session != STD_NULL ? (vm_platform_run_context *)&session->platform_run_context : STD_NULL; }
vm_platform_run_handle *vm_session_fixture_platform_run_handle(const vm_session *session)
{ return session != STD_NULL ? (vm_platform_run_handle *)&session->platform_run_handle : STD_NULL; }
t_bios *vm_session_fixture_default_bios(const vm_session *session)
{ return session != STD_NULL ? (t_bios *)&session->default_bios : STD_NULL; }
const vm_profile_default_pc_at_descriptor *vm_session_fixture_profile(
    const vm_session *session)
{ return session != STD_NULL ? session->profile : STD_NULL; }
