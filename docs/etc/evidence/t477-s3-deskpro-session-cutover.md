# T477 S3 DeskPro Session Core-Input Cutover

`M5:T477:S3:DESKPRO-SESSION-CUTOVER:OK`

`vm_session_create_model40_byob` now resolves and stores the copied DeskPro
child result before constructing session resources. `core_machine_config` and
controller timing rules are copied only from that result. The former direct
session call to `vm_profile_model40_core_config_initialize` is deleted; that
initializer remains solely inside the child-declaration producer, so there is
one immutable Model-40 Core-input construction path.

The session continues to own only dynamic BYOB ROM bytes, D4 backing storage,
mounted media, registry/display/debugger lifetime and the FDC observation
callback. No mutable resource enters resolver data and no Core code selects a
DeskPro name.

`vm-model40-private-composition-s7-smoke` proves the resolved identity,
parent identity and copied Core input equal the actual session input, in
addition to its existing ROM/D4/timing checks. The integration smoke passes
unchanged. The source sweep finds `vm_profile_model40_core_config_initialize`
only in the child declaration producer and its private declaration, never in
session construction. The full current-gate build passes with 294 targets.

Tracked runtime/test delta excluding documentation is +18/-1 lines across
three paths: one copied resolved value in session storage, nine session lines
that replace the direct initializer call, and eight parity assertions/marker.
