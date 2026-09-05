# T522 S1 Shared-Library Migration Ledger

## Evidence Basis

The owner supplied the staged `src/lib/platform/win32` corpus and authorized
its use on 2026-09-05. It byte-matches
`O:/repos.hobby/softpc/src/lib/platform/win32` at source revision
`ff3b21a36180ff8d3b7efa0feac7bc068c7224fc`; the local
`MANIFEST.sha256` records the normalized source inventory. A complete staged
file scan found no independent copyright, SPDX, or license notice. The one
SoftPC name in a code comment is not a source attribution or a retained
product dependency. S2 must add the admitted source paths, revision, owner
authorization, retained notices, changes and verification to the provenance
index before any import commit.

The audit covered all staged files, all current `src/vm/platform/win32` and
`src/vm/platform/linux` files, `src/core/platform/{win32,linux}`, the VM
session-composition tree, configured CMake platform sources, and their direct
unit/integration callers. `src/lib` is currently untracked and is not compiled.

## Frozen Dispositions

| Item | Current owner | S2 disposition | Reason |
| --- | --- | --- | --- |
| `frame.h`, `event.h`, `router.[ch]`, `presenter.[ch]` | staged Win32 component | `lib/ux/`, redesigned as platform-neutral value/binding contracts | Already contain only copied presentation values or generic router/binding intent. |
| `actions.[ch]` | staged Win32 component | split: neutral action/chord values in `lib/ux/`; Win32 virtual-key conversion/matching in `lib/ux/win32/` | `actions.h` currently exposes `WORD` and `<windows.h>`, forbidden at the root UX ABI. |
| `mailbox.[ch]`, `event_queue.[ch]` | staged Win32 component | neutral mailbox/queue semantics in `lib/ux/`; Win32 lock/event storage and waits private to `lib/ux/win32/` | Current APIs expose an event as `void *` and implementation owns `HANDLE`; no public platform handle may survive. |
| `console.[ch]`, `window.c`, `input.[ch]`, `mouse.[ch]`, `geometry.[ch]` | staged Win32 component | `lib/ux/win32/` | Native Console/Window loop, GDI, RDP input, capture and geometry are Win32-only UX implementation. |
| `README.md`, `MANIFEST.sha256` | staged Win32 component | replace with `src/lib/README.md` and an updated layout-aware source manifest | The old directory-specific declaration becomes false after the split. |
| `vm/platform/win32/{win32,win32app,win32con,w32adisp,w32cdisp}.[ch]`, `win32_types.h` | NXVM VM platform | S4 replacement/deletion after `lib/ux/win32` owns all equivalent routes | These are duplicate native presenter/input/router paths once the library is selected. |
| `vm/platform/linux/{linux,linuxcon}.[ch]` | NXVM VM platform | S2 migration candidate into `lib/ux/linux`, then S4 replacement/deletion | It is the current Linux Console loop and consumes VM/Core objects directly; S2 must bind it through copied UX values. No Linux Window backend is implied. |
| `core/platform/{win32,linux}/sleep.c`, `core/platform/sleep.[ch]`, `wait.[ch]` | Core platform | retain in Core | Core timing/wait behavior is a Core contract; moving it would give `lib/host` a Core dependency or create duplicate waits. |
| `core/platform/win32/keyboard.[ch]` | Core platform | retain in Core pending a separate input-boundary decision | It maps input to Core protocol; `lib/ux` emits only neutral host events. |
| `vm/composition/session/{control,execution,lifecycle,runner}.[ch]` | NXVM VM session composition | retain as the current product owner; admit only a new generic lifecycle mechanism after an exact callback/proof map | Every current implementation references `vm_session`, Core reset/debug, profile/provider lifecycle or runner semantics. It cannot be mechanically moved. |
| remaining `vm/composition/session/*`, including `media.[ch]`, ROM, machine and Model-40 composition | NXVM VM session composition | retain product-local | These own profiles, firmware assets, controller/media topology and product behavior, not generic storage/session mechanics. |
| `vm/platform/{execution,host_surface,input_flush,request_bridge,virtual_time,vm_request_transport}.[ch]` | NXVM VM platform | retain unless S3 ledger proves a complete neutral replacement | They carry VM requests, Core execution, guest input and product-specific timing semantics. |
| generic `lib/host`, `lib/session`, `lib/storage`, `lib/observability` roots | none | create only for an admitted finite capability | No staged implementation currently qualifies for these roots. Empty scaffolding is prohibited. |

This table exhausts the 23 staged files: the first four rows contain all
21 implementation/header files; the fifth contains the two metadata files.
Every current NXVM candidate is retained, moved/replaced, or excluded above.

## Required Independent Contracts

| Component | Sole responsibility | Must not know or depend on |
| --- | --- | --- |
| `lib/ux` | copied frame/event values, action registration, display routing, presentation binding, and native UX loops | `host`, `session`, `storage`, `observability`, Core/VM objects, guest protocol, profile or firmware |
| `lib/host` | separately consumable host synchronization values and implementations | `ux`, `session`, `storage`, `observability`, Core/VM objects |
| `lib/session` | product-neutral serial lifecycle transition state and bounded executor request/result | `ux`, `host`, `storage`, `observability`, a VM/machine/profile/guest executor pointer |
| `lib/storage` | generic image-byte and copy-on-write overlay capability | every peer component and controller/drive/media policy |
| `lib/observability` | bounded typed outcome and diagnostic-event values | every peer component and execution control |

Each component is an independent build unit. Native UX sources may include
their own root UX headers, but no top-level component includes another. A
product composition root is the only place allowed to combine them.

## S2 Design Constraints

1. UX owns complete Console/Window event loops. A mailbox's wake mechanism is
   private to the selected UX native implementation, not a `host/sync`
   dependency and not a public `HANDLE`/fd leak.
2. The root action contract uses project-defined value keys. Win32 maps native
   `WM_KEY*`/Console records to those values; no Windows type appears in a
   root header.
3. The one permitted process-global UX state is a Console lease with no
   product/session/machine pointer. Every Window's rendering, input, capture,
   cursor and close state is allocated per presenter.
4. Product callbacks use opaque context plus copied values and typed outcomes.
   They never export a raw Core, VM, CPU, RAM, device, executor or session
   pointer through a public library contract.
5. `lib/session` does not own a host thread. Its executor callback is invoked
   by the product-selected runner, which retains host scheduling and guest
   execution ownership.

## Admission Result

There is no source-license, ownership, or required-product-capability stop.
However, a direct relocation is rejected: the staged Win32 component is not
currently a cross-platform UX library, and no present source implements the
four proposed non-UX components. S2 may perform the bounded UX split and Linux
UX conversion only. S3 must first admit a separate finite capability ledger
for every `host`, `session`, `storage`, or `observability` file; it may not
create a directory merely to satisfy the target diagram.
