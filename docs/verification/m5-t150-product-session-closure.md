# M5 T150 Product-Session Closure

## Scope And Result

NXVM shared product-session management is closed through T150. A successful
`core_product_session_manager_create()` now creates and selects opaque session
0 before it returns. Consequently no product-visible manager can have zero
live sessions or no selected session. The sole public close path rejects the
final session before it calls a concrete provider.

The implementation has one owner chain:

```text
vm/main -> core_product_session_manager -> VM session provider -> vm_session
        -> VM selected-machine provider -> retained NXVM Console commands
```

The manager is generic and contains no VM/VDM include or concrete guest
operation. VM composition owns concrete create/describe/stop/join/finalize/
destroy. The Console owns only an explicit manager reference and output
callback; it does not retain an ID or `vm_session*`.

## Static Closure

`verify-product-session-manager` printed
`M5:T150:PRODUCT-SESSION-MANAGER:OK`. It verifies atomic session-0 creation,
final-session rejection, complete shared grammar, absence of core-to-VM/VDM
includes, no mutable manager static state, and removal of old Console-target
vocabulary.

The established dependency, single-executor, facade-ownership,
session-readiness, and C-facade gates also passed. The only remaining mutable
inventory entries are the already documented explicit process-exclusive Linux
terminal and Win32 Console leases.

## Runtime Closure

- Core manager smoke: `M5:T146:S1:SESSION-MANAGER:OK`.
- Real VM manager smoke: `M5:T147:S1:VM-SESSION-MANAGER:OK`.
- Two real Window sessions: `M5:T149:S1:MULTI-WINDOW:OK`.
- Host-surface lease: `M5:T89:S1:HOST-SURFACE-LEASE:OK`.
- FDD boot/DOS prompt: `M5:T70:S2:DOS-PROMPT:OK`.
- Retained Console lifecycle: `M5:T96:S1:CONSOLE-LIFECYCLE:OK`.

The final artifact transcript proved `LIST`, `OPEN`, `SELECT`, ordinary
`CLOSE`, and the exact final-session rejection. Its artifact is
`build/output/nxvm_0_5_0150.exe`; SHA-256 is
`947F56A8735AA097B715F6A36870732381E32017357F46ADFD310AE851FB9B32`.

## VDM Adoption Boundary

VDM does not adopt this implementation in M5. It remains a non-runnable,
isolated skeleton. M6/M8/M9 must first define the owned DOS session, debug
Console/CLI contract, executable loading, cancellation, and VDM provider
lifetime. Only then may `vdm/composition/session` provide the same opaque
provider/selected-machine bridge for debug mode. Non-debug `ntvdm64 run` will
still create exactly one manager session 0 internally, but it must not expose a
management Console. No DOS runtime, host filesystem route, CLI behavior, or
Windows integration is introduced by this closure.
