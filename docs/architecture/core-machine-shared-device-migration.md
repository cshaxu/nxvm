# Core Machine Shared-Device Migration

## M5 T296 S1 Decision Record

This matrix is the approved design boundary for T296 only. It is an audit of
the T295 source baseline, not a new public API or behavior change. One
`core_machine` remains the mutable guest-state owner. VM composition selects
the immutable profile and submits frozen typed topology, configuration, and
providers while `INITIALIZED`; it neither borrows controller/PIC/port storage
nor retains a second lifecycle owner. A provider may retain only its documented
context through core-controlled teardown.

The source audit used:

```powershell
rg -n "core_machine_(configuration|profile_binding|debug)_" src tests --glob '*.[ch]'
rg -n "(vadp|dma|rtc|cmos|nmi|fdc|hdc|port).*(_initialize|_reset|_finalize|bind|install)" src/core src/vm --glob '*.[ch]'
```

The retained lifecycle is `create -> configure/bind while INITIALIZED -> freeze
-> reset/run -> destroy`. In a completed stage, core performs initialization,
binding, reset, and finalization of the named shared device; composition only
submits the frozen typed value/provider. The existing media registry/provider
route remains the only media path. Backing and path policy remain VM
composition-owned and do not move into core.

## Fixed Stages

| Stage | Bounded responsibility | Permitted VM/profile input after migration | Explicit exclusion |
| --- | --- | --- | --- |
| S2 / A | VADP and port initialization, registration/binding, reset, and finalization | typed display timing/EGA configuration, declared port topology, display provider | RTC, DMA, FDC, HDC, firmware-capability redesign |
| S3 / B | DMA plus RTC/CMOS/NMI initialization, registration/binding, reset, advance, and finalization | typed DMA/IRQ routes, RTC tick rate, CMOS default bytes, declared CMOS ports | FDC/HDC connection or media binding |
| S4 / C | FDC/HDC initialization, controller binding, port registration, reset, and finalization | frozen controller ports/IRQ/DMA route, drive-slot identity, media/provider policy | media backing ownership/path policy, firmware/debug capability redesign |

## Migration Matrix

| Group and current real call | Current owner and VM raw exposure | Typed replacement and core lifecycle entry | Regression / stage acceptance |
| --- | --- | --- | --- |
| **A - port bus.** `core_machine_create` calls `core_machine_port_initialize` and `core_machine_memory_register_ports`; cold reset calls `core_machine_port_reset`; destroy calls `core_machine_port_finalize`. `vm_session_machine_devices_initialize_cmos` and `vm_session_machine_devices_initialize_hdc` still call `core_machine_install_port_provider`. | `core_machine.executor_port` is embedded core storage. VM does not borrow it for RTC/HDC registration, but their registration remains imperative VM lifecycle work. FDC separately borrows it and is reserved to C. | S2 introduces a frozen typed port/topology submission for display-owned registration, consumed by core during `INITIALIZED`; generic core port lifecycle stays in its existing create/reset/destroy entries. B and C port submissions wait for their stages. | S2 static gate rejects VM display-port installation or port borrow; retain VADP text/status, CGA/EGA, ROM/Console, and port corpus. Stop before B/C registration is altered. |
| **A - VADP.** `core_machine_create` calls `core_machine_vadp_initialize(&shared_vadp, &executor_port)`; reset/destroy call `core_machine_vadp_reset/finalize`. `vm_session_create` invokes three `core_machine_profile_binding_configure_*` VADP operations from profile values. | `core_machine.shared_vadp` is embedded core storage; its callbacks bind directly to the embedded port bus. VM has no VADP pointer, but profile binding directly mutates VADP/RAM. | Replace VADP profile-binding configurators with one frozen typed display-topology/configuration submission. Core validates, applies, and owns it before provider freeze, retaining reset/finalize order. Capture remains copied through `core_machine_capture_display_snapshot`. | S2 retains VADP text/status, CGA, EGA sequencer/controller/planar, ROM-video, Console, and isolation results. Stop if firmware requires an opaque capability: T297 owns it. |
| **B - DMA.** `core_machine_create` calls `core_machine_dma_initialize`; cold reset/destroy call `core_machine_dma_reset/finalize`. `vm_session_machine_devices_initialize_fdc` borrows DMA latch/primary/secondary and calls `core_machine_dma_bind_channel`. | DMA storage is embedded in `core_machine`; FDC composition exposes all three raw DMA borrows. The typed `core_machine_dma_request_binding` already prevents FDC retaining raw DMA storage after connection. | S3 adds frozen typed DMA wiring submission; core invokes channel binding while configuring its own state. S3 does not connect FDC or consume media; its endpoint is S4. | S3 static gate rejects VM DMA storage borrows/bind call; retain DMA-channel, RTC/CMOS/NMI, timer/IRQ/boot regressions. Stop if FDC/HDC connection changes. |
| **B - RTC/CMOS/NMI.** `vm_session_machine_devices_initialize_cmos` calls `core_machine_rtc_initialize`, applies NVRAM defaults, and installs a VM RTC port provider. VM execution-provider `advance` calls `core_machine_rtc_advance`; reset/finalize call `core_machine_rtc_reset/finalize`. Port `70h` NMI-mask writes call `core_machine_set_nmi_mask`. | `vm_session.rtc` is remaining VM-owned shared-device storage. RTC receives borrowed master/slave PIC storage; the CMOS port callback is VM-owned mutable-state access. NMI mask already uses a typed core operation. | S3 moves RTC storage and port-provider implementation to core. VM/profile supplies a typed CMOS port pair, IRQ/tick route, default bytes, and NMI wiring declaration. Core owns RTC initialize/reset/advance/finalize and port `70h/71h`; no host time. | S3 runs CMOS/RTC boundary and port corpus, RTC/CMOS/NMI, timer/IRQ, boot, Console/debugger, and isolation regressions. Stop if firmware operations need redesign; T297 owns that contract. |
| **C - FDC.** `vm_session_machine_devices_initialize_fdc` borrows core FDC, DMA, PIC, and port storage; it binds DMA, calls `core_machine_fdc_connect`, then `core_machine_fdc_initialize`. Core cold reset/destroy already call `core_machine_fdc_reset/finalize`. | `core_machine.fdc` is embedded core storage; VM raw borrows compose it. VM-created media registry remains the only provider route; frozen drive-slot IDs are already typed. | S4 replaces borrows with a frozen FDC topology: ports, IRQ/DMA wiring, typed drive slots, and registry/provider binding. Core performs DMA bind, FDC connect/initialize/reset/finalize, and port registration. VM retains backing/media policy. | S4 static gate rejects VM FDC/PIC/DMA/port borrows and direct lifecycle calls; run core media-I/O and FDC ports plus FDD/DOS, boot, Console/debugger, and isolation regressions. Stop on a second media route or backing/path migration. |
| **C - HDC.** `vm_session_machine_devices_initialize_hdc` borrows core HDC and PIC storage, calls `core_machine_hdc_connect/initialize`, then installs its port provider. Core cold reset/destroy already call `core_machine_hdc_reset/finalize`. | `core_machine.hdc` is embedded core storage; VM borrows HDC/PIC and registers ATA port ranges. `vm_session.hdd` and its backing stay VM-owned and use the existing media registry. | S4 replaces borrows with frozen typed ATA topology: PIO ranges/features, IRQ, abstract drive slot, and media/provider policy. Core validates neutral topology, connects/initializes/registers/resets/finalizes HDC, and receives no local path, PC/AT identity, or BIOS/DOS meaning. | S4 static gate rejects VM HDC/PIC/port lifecycle wiring; run core media-I/O, ATA/FDC port, HDD/FDD/DOS, boot, Console/debugger, and isolation regressions. Stop if policy enters core or a raw firmware/debug capability is required. |

## S2 / A Implementation Evidence (Pending Coordinator Review)

`core_machine_display_config` is the sole S2 typed submission: neutral text
timing, EGA sequencer/controller configuration, exact VADP port groups, and a
typed display-provider slot. `core_machine_configure_display` accepts it only
while configuration is open, validates the fixed VADP topology, applies the
configuration to embedded VADP/executor RAM, copies the topology, freezes the
provider, and rejects a second submission. Core's existing VADP/port create,
reset, and finalization entries are retained. The copied display snapshot
boundary is unchanged.

`vm_session_storage_initialize` now binds the composition-owned typed provider
slot and submits profile timing/EGA values plus declared port ranges through
that core entry. The former three profile-binding VADP configuration calls are
removed. No DMA, RTC/CMOS/NMI, FDC, or HDC source changed. In particular,
`machine_devices.c` still contains RTC and HDC direct port installation; those
known B/C hits are intentionally deferred rather than hidden by the A-only
static gate.

Evidence: the focused `core-machine-display-authority-smoke` proves accepted
submission, provider freeze, freeze-time rejection, reset, and EGA aperture
write (`M5:T296:S2:DISPLAY-AUTHORITY:OK`); `verify-core-display-authority`
rejects raw VADP/profile-binding/port-install calls in the display-composition
sources. The managed `current-gates-gcc` run passed all 46 static/build gates
and 123 current-gate tests, including VADP text/status, CGA/EGA, ROM-video,
Console, and two-session isolation. Media-dependent tests used only existing
owner-provided fixtures selected in the untracked build cache. A compiler
warning about the pre-existing VADP EGA CRTC index-`13h` storage bound is
tracked as `TODO(High)` for a future focused VADP admission; it is not changed
by S2.

## Preserved Ordering and Deferred Edges

The current VM sequence is recorded, not yet moved: media objects, profile
firmware, CMOS initialize/default/reset, keyboard/DMA firmware hooks, FDC,
HDC, media binding, FDC/HDC firmware hooks, and core POST hooks. Provider reset
currently precedes profile-firmware reset; finalization runs profile firmware
before VM device finalization. S2, S3, and S4 preserve observed order at their
boundary and prove it with focused lifecycle evidence; they do not reorder
another group.

Deferred edges are intentional: profile firmware raw binding and port writes
are T297; debugger borrows and UX are T298; remaining public raw configuration
borrow deletion is T299. This task neither closes nor weakens their contracts.

## Stage Stop Conditions

- **S2:** stop before DMA, RTC/CMOS/NMI, FDC, or HDC implementation changes.
- **S3:** stop before FDC/HDC connection, media registry binding, or drive/media
  policy changes.
- **S4:** stop before firmware/debug capability work, media backing/path-policy
  relocation, or a new media route.

Every stage requires a narrow owner static gate and focused lifecycle regression
before the next stage. After all stages, the task must build and preserve
`build/output/nxvm_0_5_0296.exe`, record SHA-256 and source commit, and pass
documentation governance, `git diff --check`, and `current-gates-gcc`; S1 does
not create an artifact.
