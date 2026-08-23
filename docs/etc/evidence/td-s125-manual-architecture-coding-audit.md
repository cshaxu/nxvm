# Td S125 Manual Architecture And Coding Audit

Status: closed audit evidence. This supporting record does not define an
architecture, coding rule, task order, or product claim. The controlling
authorities are linked from [docs/README.md](../../README.md); the active
packet is [CURRENT.md](../../states/CURRENT.md).

## Method And Boundary

The review was a manual reading of the declared authority set, central
production headers/implementations, representative creation/failure/cleanup
paths, CMake target graph, and tests that consume those paths. Structural gates
were used only to corroborate the source reading:

- `tools/Verify-DependencyDag.ps1` reports no known migration edge.
- `tools/Verify-CFacadeHeaders.ps1` reports `M5:T113:C-FACADE-HEADERS:OK`.
- The documentation gate passed again at closure; the raw-borrow verifier also
  passed, with its concrete coverage limit recorded as finding I.

These results do not establish conformance by themselves. This record labels a
component `conforming` only for the property actually reviewed; it makes no
whole-program behavior or execution-correctness claim.

## Manual Disposition Matrix

| Area | Files manually traced | Disposition |
| --- | --- | --- |
| Principal documentation | `docs/README.md`, Rules, Design, `CURRENT.md`, `TODO.md`, `etc/README.md` | Authorities and current source-map intent are internally consistent. The earlier Td S124 record was corrected in `CURRENT.md`: its static checks were limited, not a full manual audit. |
| Global type foundation | `src/type.h`, `src/type.c` | The C facade is the designated root foundation and its platform calls remain isolated in `type.c`; no new architecture finding from its legacy vocabulary. Its raw pointer-conversion helpers were observed as CPU/memory implementation plumbing, not a public cross-module contract. |
| Core machine | `machine_interface.h`, `machine.h`, `machine.c`, timing, plan, firmware, device and lifecycle sections | `core_machine` remains opaque at the intended public facade and Core has no direct VM/VDM source include. The coordinator implementation is nevertheless non-cohesive; see finding A. |
| Core platform/product | session manager interface/provider/manager, debug target | The debug target is a bounded injected capability. The session manager leaks untyped selected-session objects; see finding B. |
| VM composition | session interface/layout, lifecycle, factory, console adapter, display, media, execution, model-40 composition | The declared composition root is present, but its public/private boundary is bypassed by direct layout coupling; see findings C, D, E, and H. |
| VM machine/profile | FDD/HDD, default PC/AT profile, Model-40 profile, default firmware headers | Media and profile implementation layouts cross module boundaries; see findings C and D. |
| VM platform | platform, request/execution/virtual-time, Linux/Win32 consumers | Host adapters remain VM-owned and were not found to enter Core. Public run-context/run-handle layouts expose host-adapter representation; see finding E. |
| VDM skeleton | minimal machine, session wrapper, presentation, CMake targets | CMake exposes libraries and smoke targets only, not an NXVDM product executable, consistent with the current non-runnable-skeleton declaration. The session wrapper is a pure forwarding layer; see finding F. |
| Tests | Core fixtures; product, platform and machine integration smokes named below | Same-owner Core setup was kept distinct from external test access. Product/platform and VM integration tests directly inspect private state; see finding H. |
| Build declarations | `CMakeLists.txt` target/dependency declarations, Core include sweep | Core has no direct include of VM or VDM source. VDM is library/smoke-only. The target graph does not cure header-layout coupling, so the structural DAG pass is not a contrary finding. |

## Confirmed Findings And Transfers

| ID | Finding | Evidence | Transfer |
| --- | --- | --- | --- |
| A | Core machine coordinator is not a cohesive owner-local implementation. | `src/core/machine/machine.c` combines timing, plan validation, scheduling, firmware, board-device configuration, lifecycle and fault publication across about 7,000 lines. | `TODO.md`: Core machine coordinator decomposition. |
| B | Core product session manager leaks a raw cross-module session object. | `core_product_session_manager_borrow_selected(..., C_VOID **out_session)` and provider `open` return raw session objects; `console_machine_adapter.c` casts them to `vm_session *`. | `TODO.md`: Session-manager raw-object escape removal. |
| C | VM session/media headers expose mutable implementation state and legacy public names. | `session.h` embeds Core, media, debug, platform and profile state; `fdd.h`/`hdd.h` publish `t_fdd`/`t_hdd` fields and backing pointers. | `TODO.md`: Cross-module VM contract boundary normalization; Legacy VM media type/public-name retirement. |
| D | VM profile headers expose implementation-owned configuration/device state. | `pc_at_profile.h` publishes the full topology representation; `model40.h` publishes writable D4 memory, parity state, and `core_machine *`. | `TODO.md`: VM profile implementation-boundary repair. |
| E | VM platform run contracts disclose mutable host-adapter representation. | `platform.h` exports run context/handle fields for native surfaces, renderers, execution transport, backend and state. | `TODO.md`: VM platform adapter contract encapsulation. |
| F | VDM session is a pure forwarding layer. | `src/vdm/composition/session.c` allocates/checks then forwards its public operations to `vdm_machine_dos_minimal_*`. | `TODO.md`: VDM skeleton forwarding-layer disposition. |
| G | The previous static-only governance closure overstated its scope. | Td S124 claimed a full static audit; its actual checks were dependency and C-facade checks. | Corrected in `CURRENT.md`; this record supplies the distinct manual evidence. |
| H | Product/platform and integration tests consume private implementation fields. | `nxvm_default_profile_smoke.c`, `vm_session_media_lifecycle_s3_smoke.c`, `vm_model40_hdc_s26_smoke.c`, `vm_multi_window_session_smoke.c`, and many `tests/machine/vm_*` tests access `vm_session`, `core_machine`, media or platform fields directly. | `TODO.md`: Product and platform test-boundary repair. |
| I | The named public raw-borrow structural verifier has a coverage blind spot. | `VerifyPublicRawBorrowClosure.ps1` passes because it searches selected historical Core borrow/layout spellings, not `C_VOID **out_session` or the VM cast path in finding B. | `TODO.md`: Public raw-borrow verifier scope repair. |

## Explicit Non-Findings

- No Core-to-VM/VDM source include, reverse dependency, or platform-SDK include
  was found in the reviewed Core machine/product/utils paths.
- No current VDM executable/product path was found; its CMake registration is
  limited to static libraries and minimal smoke tests.
- `vm_product_console_machine_provider` and `core_product_debug_target` are
  injected capability tables with named operations; their opaque contexts are
  not, by themselves, evidence of a raw Core/VM object escape.
- Legacy implementation names inside an owner (for example Core device test
  fixtures or Win32-local symbols) were not classified solely by naming. A
  finding requires a real cross-owner layout/ownership or responsibility leak.

## Review Limit

This audit records architecture and coding-rule conformance, not an exhaustive
functional, security, source-provenance, or hardware-accuracy certification.
The closure check reran safe structural/documentation gates and verified that
every unresolved finding above has a bounded TODO transfer.
