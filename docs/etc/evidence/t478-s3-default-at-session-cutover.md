# T478 S3 Default-AT Session Cutover

`M5:T478:S3:DEFAULT-AT-SESSION-CUTOVER:OK`

## Retained Route And Deleted Route

`vm_session_create` now converts the supplied Default-AT configuration once to
the private typed request, resolves the `default-at` child, and copies only its
frozen Core configuration/controller timing plus copied descriptor snapshot
into the session. Core plan creation follows the existing common session path.

Deleted from the Default-AT production path are the static descriptor selector,
direct Core materializer, direct CPU/FPU contract selector and post-copy memory
override. The generic root and DeskPro child routes remain separate only where
their declared semantic/lifetime differences require it.

The session still owns media paths/bytes, display and input bindings, debugger,
run control, boot preference and product lifetime. The resolver owns immutable
identity/provenance/configuration; Core owns all mutable machine/device state.

## Parity And Sweep

The default session now reports resolved identity `default-at` rather than the
former static descriptor identity `default-pc-at`; command-line request spelling
and `vm_session_profile_name` remain unchanged. Focused tests preserve default
and overridden CPU/FPU/memory configuration, ATA/HDC/CGA topology, initialization
rollback and the 5170 HDC-negative route.

The S3 sweep:

```text
rg -n "vm_session_profile_select|vm_session_materialize_profile_core_config|vm_session_apply_core_config_overrides|vm_session_cpu_contract_select" src tests
```

has no hits. The only remaining `vm_profile_default_pc_at_core_config_materialize`
call is the shared profile-value construction helper, used before resolution;
there is no Default-AT session call site.

Tracked source/test delta, excluding status/evidence, is `+35/-60`, net
`-25` lines over `session.c`, `session_private.h` and the composition smoke.
The retained production path is the one request-to-resolved-child conversion;
the net reduction is the removed direct materialization/override machinery.

## Verification

Focused construction corpus passed:

```text
vm-session-initialization-atomicity-smoke
vm-ibm-5170-model-339-composition-smoke
vm-default-pc-at-profile-smoke
vm-pcat-composition-s4-smoke
vm-hdc-port-smoke
```

`cmake --build --preset current-gates-gcc` passed all 257 steps, including
source/dependency/ownership checks and documentation governance. The retained
T345 negative fixture reports its expected internal error and then passes its
self-test.
