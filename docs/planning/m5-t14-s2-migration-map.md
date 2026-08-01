# M5 T14 S2 Dependency Migration Map

## Scope

This is the current design-only migration map for M5 T14. It records the
actual remaining ownership debt and the order for removing it without changing
the retained NXVM Console, debugger, boot sequence, media behavior, or
developer artifact identity. `contracts.md` and `module-layout.md` remain the
interface and ownership authorities; this file turns them into M5 work.

No row authorizes a runtime move. A row stops for owner approval when its
proposed adapter changes observable ordering or guest behavior.

## Target Graph

The final CMake graph contains one static target for each implemented module:

```text
core-machine       core-platform       core-product
      ^                   ^                  ^
      |                   |                  |
vm-machine       vm-platform         vm-product       vm-profile
      \                |                 /               /
       +---------------+----------------+---------------+
                            vm-composition -> nxvm.exe

vdm-machine      vdm-platform        vdm-product       vdm-profile
      \                |                 /               /
       +---------------+----------------+---------------+
                            vdm-composition
```

`vm-composition` and `vdm-composition` are the only targets allowed to link
their product form's peer modules. M5 keeps `vdm-composition` test-only: it
does not create a user-facing ntvdm64 executable. Existing task artifacts must
link the same `vm-composition` graph as `nxvm.exe`; they may not compile an
independent hand-selected source list.

## Shared Gates

Every runtime slice configures and builds with GCC. The smallest applicable
gate runs first; an affected full-PC slice also runs the retained Console,
debugger, CPU, FDD/HDD reset-vector, and bounded fixture gates. No raw recorder
is enabled.

| Gate | Existing evidence or target |
| --- | --- |
| Core | `nxvm-core-*-smoke`, especially lifecycle, CPU capability, debug, presentation, and PC-device smokes |
| Transport | `nxvm-platform-vm-request-bridge-smoke` and `nxvm-vm-request-transport-smoke` |
| VM composition | `nxvm-vm-full-pc-session-smoke`, `nxvm-full-pc-profile-smoke`, and `nxvm-default-profile-smoke` |
| VDM shell | `ntvdm64-vdm-minimal-session-smoke` and `nxvm-dos-minimal-presentation-smoke` |
| Retained UX | existing no-media Console and delayed debugger gates recorded through M5 T13; full-PC changes also retain FDD/HDD reset-vector and bounded fixture evidence |

## Source-Edge Map

| Current source edge | Required boundary and migration action | CMake destination | Gate and stop condition |
| --- | --- | --- | --- |
| `core/machine/{vcpu,vcpuins,vdma,vpic,vpit,vport,vram}.c -> core/product/utils.h`; `core/machine/vglobal.h -> core/platform/global.h` | Remove legacy global utility/type coupling. Migrate shared types, C-runtime wrappers, and legacy trace primitives to the approved root `type.*`; keep assembler wrappers and product-only helpers in `core-product`. | root foundation, `core-machine`, `core-product` | Core. Stop if legacy diagnostics, CPU trace, allocation, or type width changes. |
| `core/platform/presentation.h -> core/machine/{presentation,status}.h` | Remove the core-platform text-snapshot wrapper. A VDM-private snapshot embeds the core text snapshot; platform copies that embedded core child object only. Root composition translates snapshot to a platform frame at the execution boundary. `core/platform` must not expose a typed core text snapshot or include `core/machine`. | `core-platform`, `vm-composition`, `vdm-composition` | Core presentation + VDM shell + injected platform-sink spy. Stop if copied text/frame fields or generation timing change, or if platform code requires a machine include. |
| `core/product/{utils.h,debug/xasm32/*.h,runtime/registry.h} -> core/{platform,machine}/*` | Move shared primitive types/status to root `type.*`; make assembler/disassembler pure-data and registry generic. Core product cannot carry CPU capability or machine status definitions. | root foundation, `core-product` | Core debug/trace/registry. Stop if assembler output, debugger display, or registry behavior changes. |
| `core/machine/vcpuins.c -> vm/machine/device.h` (`deviceStop`) | Replace direct stop with the core-machine safe-stop/result path. VM composition maps the result to the retained VM stop state. | `core-machine`; binding in `vm-composition` | Core lifecycle/CPU + retained debugger. Stop if F9, debugger stop, or CPU exception behavior differs. |
| `core/machine/{vkbc,vdma,vpic,vpit}.c -> vm/profile/default_profile/firmware/vbios.h` | Remove direct BIOS assembly-string registration. Default profile contributes an ordered immutable firmware plan; VM composition binds the plan before freeze, preserving the legacy `vmachineInit` registration order. | `core-machine`, `vm-profile`, `vm-composition` | Core firmware + default-profile firmware + FDD/HDD reset-vector. Stop if POST or IVT order changes. |
| `vm/machine/machine.c -> vm/platform/platform.h` | Move lifecycle orchestration (`platformInit/Start/Final`) into `vm-composition`; retain machine-only reset/stop operations behind VM machine API. | `vm-machine`, `vm-platform`, `vm-composition` | VM composition + Console. Stop if `START`, `STOP`, `RESUME`, `RESET`, or Console shutdown order changes. |
| `vm/machine/{vcmos,vfdc,vhdc,vmachine}.c -> vm/profile/default_profile/firmware/*` | `vmachine.c` becomes composition-owned assembly/reset/refresh ordering. Device-specific POST/INT contributions become profile-plan entries, not device-to-profile calls. The product-neutral video-adapter state is in `core/machine`; default-profile CGA firmware consumes that core state directly. | `core-machine`, `vm-machine`, `vm-profile`, `vm-composition` | Firmware + FDD/HDD reset-vector. Stop if device initialization, reset, or refresh ordering changes. |
| Shared debugger UI | Complete: `debug.c/.h` now live in `core/product/debug` and use the typed core debug target only. VM root composition binds the retained device implementation; a future VDM composition binds the same contract. Console remains a product command target with no machine/platform include. | `core-product`, `vm-composition`, `vdm-composition` | Console + debugger + VM composition. Stop for any changed prompt, command acceptance, media behavior, display toggle, debug entry behavior, or debugger text. |
| `vm/profile/default_profile/{default_profile.*,firmware/{qdcga,qddisk,qdx,vbios}.c/.h} -> vm/{machine,platform,product}/*` | Replace sibling calls with declared provider requirements. A video-mode change invokes the product-neutral core display notification; VM composition binds it to the retained platform reconfiguration call at the same synchronous point. VM HDC binds the approved core block capability for reset-time geometry and sector operations; profile metadata does not call media/product code. | `vm-profile`, capability adapter in `vm-composition` | Default-profile firmware + FDD/HDD reset-vector. Stop if video refresh timing or geometry/sector behavior changes. |
| `vdm/platform/dos_minimal_presentation.* -> vdm/machine/dos_minimal.*` | Move session-to-presentation translation into `vdm-composition`. Platform keeps copied event/frame mechanism; VDM machine keeps DOS-minimal state. | `vdm-platform`, `vdm-machine`, `vdm-composition` | VDM shell. Stop if the no-media smoke's input/frame behavior changes. |
| `vdm/product/minimal_session.* -> vdm/{machine,profile}/*` | Move profile selection and session construction to `vdm-composition`; retain any future VDM UX in `vdm/product`. | `vdm-machine`, `vdm-profile`, `vdm-composition` | VDM shell. Stop before creating a VDM executable or changing DOS-minimal behavior. |

The direct-source scan must report every edge in this table. Same-module
includes, such as one VM machine device including another VM machine device,
are not forbidden peer edges.

## CMake-Edge Map

| Current target/debt | Required outcome |
| --- | --- |
| `nxvm-baseline-runtime` | Complete: deleted. Its former core-product, VM-profile, and VM-platform sources now compile in separate ownership targets. The temporary `nxvm-legacy-runtime-dependencies` target is link-only (`INTERFACE`) and owns no source; M5 closure replaces it with final module links. |
| `nxvm-machine-core-executor` | Complete for the retained build: it now compiles only `core/machine` sources. The final named `core-machine` target remains M5 closure work. |
| `nxvm-firmware` | Split core firmware registry from default-profile firmware implementation; composition links both. |
| `nxvm-product-default-profile`, `nxvm-product-profile`, `nxvm-product-session` | Remove product/profile cross-linking. Product UX and profile metadata become peers linked only by `vm-composition`. |
| `nxvm-vm-full-pc`, `nxvm-vm-full-pc-session` | Replace with the final `vm-composition` owner once its lifecycle bindings preserve the current session gate. |
| `add_nxvm_full_pc_artifact` | Replace its hand-selected source aggregation with linkage to the final target graph. It remains the task-artifact producer, not a second build graph. |
| VDM minimal targets | Recast as four peer targets plus test-only `vdm-composition`; no VDM executable or CLI enters M5. |

## Execution Order

1. Add automated include and CMake-target dependency checks with the current
   violations allowlisted by this map. `tools/Verify-DependencyDag.ps1`
   compares exact source-owner edges against its checked-in temporary
   allowlist; CMake also rejects a newly introduced mixed-owner target. Make
   new violations fail immediately.
2. Introduce the minimal core stop/yield and ordered firmware-plan contracts;
   bind them in VM composition without changing the legacy device sequence.
3. Move VM lifecycle, platform ingress/egress, and retained product command
   adapters into `vm-composition`; retire direct sibling calls one event kind at
   a time.
4. Remove profile-to-machine/platform calls using reviewed capability adapters;
   dissolve all mixed CMake targets and rebuild task artifacts from the target
   graph.
5. Apply the same composition split to the isolated VDM minimal shell without
   starting DOS implementation or a VDM executable.
6. Add `type.*` and `version.*`, complete naming migration, remove allowlist
   entries, and run the full source/target DAG plus retained UX gates.

## Review Points

The following are deliberate approval points rather than hidden implementation
choices:

- CPU halt returns `WAITING_FOR_INTERRUPT`; VM composition supplies host
  waiting and waking.
- VM HDC binds the core block capability used by `vbios` and `qddisk`; geometry
  is snapshotted at reset.
- Video mode changes publish the copied display frame and invoke the retained
  platform reconfiguration synchronously through composition; direct host
  callbacks remain separately gated.
- `vmachine.c` ordering moves whole into composition before any refactoring of
  its sequence or global-state lifetime.

All other source moves, include repairs, target splits, and focused gates follow
the current contracts and this map without a new product decision.
