# T483 S2 IBM PC/XT 5160-268 Capability Matrix

`M5:T483:S2:XT-CAPABILITY-MATRIX:OK`

`M5:T483:S2:XT-OWNER-PARTITION:OK`

## Method And Boundary

This is a read-only comparison of the accepted T483 S1 bill of materials with
the current Core/VM source.  `Current owner` identifies the one owner of an
already-existing capability; `absent` means no XT-capable owner currently
exists.  It is not permission to bind an AT mechanism as XT hardware.

The first functional receiver below means the next queued XT selected-device
functional closure.  Its one construction flow must be:

`immutable XT profile -> copied Core plan -> existing or newly-required Core owner -> copied presentation/media observation`.

It must not introduce an XT global, a second controller state, an ATA shim for
Xebec, or a second video/media cache.  T483 S3 alone receives timing source
partitioning; no row below makes a timing or L3 claim.

## Capability Matrix

| Selected BOM capability | Source-labelled baseline | Current capability and sole owner/absence | Functional gap and earliest receiver |
| --- | --- | --- | --- |
| Profile identity and construction | S1: IBM 5160-268, 256 KiB, enhanced keyboard and one half-height diskette drive. | **Absent.** `vm_session_profile_kind` and VM composition enumerate only default PC/AT, IBM 5170 Model 339 and Model-40. | Add one immutable `ibm-5160-268` resolver/profile descriptor and copied Core plan.  It must reject AT-only options rather than carry mutable per-session machine selection. |
| CPU attachment | S1: 8088 maximum-mode 4.77 MHz; reuse 8086 instruction semantics, not its board attachment. | **Absent.** `core_machine_cpu_profile` has 8086/80186/80286/80386 only; the timing model selects no 8088 profile. | Add an 8088 CPU/profile attachment to the existing CPU owner, sharing semantic decode only where the retained 8086 ledger permits it.  The receiver must separately select bus/prefetch behavior. |
| 256 KiB RAM, ROM sockets and reset entry | S1: 256 KiB baseline; two external 32 KiB ROM devices provide POST/I/O/graphics/diskette bootstrap. | Generic mapped memory, immutable ROM and real-mode entry already belong to Core; no XT descriptor selects their layout. | The new profile owns frozen RAM/ROM declarations and external-BYOB validation.  It must not embed ROM bytes, path, digest or a second reset path. |
| Single PIC, PIT, DMA and refresh board route | S1: IRQ0 timer, keyboard IRQ1, eight IRQ levels, three external DMA channels plus refresh, PIT channel-1 refresh/DMA request. | PIC, PIT and DMA state are each Core-owned, but `pc_at_profile.c` binds the dual-PIC/AT route and AT DMA topology. | Construct the documented XT routes through the existing controller owners or add only the missing topology capability to those owners.  No reuse of the PC/AT descriptor and no profile-owned IRQ/DRQ state. |
| Keyboard | S1: enhanced keyboard selected; the board has a serial keyboard interface. | Current keyboard controller owner is the AT-oriented KBC route (`60h`/`64h`, including AUX), not an XT-qualified serial/PPI attachment. | Add one XT keyboard-board binding behind the existing input/controller boundary after its source-qualified interface is frozen.  Mouse/AUX remains unselected. |
| Diskette | S1: one half-height drive; media characteristics are not selected by the catalogue. | Core FDC owns current 8272A controller/media state, while the available profile binding is PC/AT-oriented. | Freeze only a source-qualified XT drive/media declaration, then bind it through the existing FDC/media owner.  Do not substitute the current AT 1.44 MB route or create a second media store. |
| CGA display | S1: IBM CGA is the explicit profile choice; `B8000h` aperture is IBM documented. | VADP is the sole Core video/VRAM/snapshot owner and has source-backed CGA capability; no XT profile selects its adapter map. | Bind the selected CGA ports and aperture through the one VADP plan route.  Presentation continues to consume only a copied snapshot; firmware or VM may not own display state. |
| IBM 20 MB Xebec expansion | S1 and T479 S6: 8-bit byte stream at `320h`--`323h`, DMA3, IRQ5 and external `C8000h`--`C9FFFh` option ROM. | **Absent.** Core HDC protocols are ATA PIO, Compaq WD 40 MB and IBM WD1003/ST-506; the current HDC contract is task-file shaped. | Add one explicit Xebec personality in the sole Core HDC/media owner, with a frozen, source-qualified BYOB geometry declaration before session open.  It must not use ATA ports, share task-file state, cache media outside HDC, or internalize the option ROM. |
| NMI, reset and 8-bit I/O channel | S1: parity NMI, `RESET DRV`, AEN and 8-bit-channel ownership are documented. | Core machine board/reset and port routing exist, but the selected current bindings are PC/AT compatibility bindings. | Add the XT board binding in the copied plan; retain reset, NMI and bus ownership in Core.  A profile supplies construction values only and cannot mutate them at runtime. |
| RTC/CMOS and AT-only devices | S1 selected BOM has no RTC/CMOS, PS/2 mouse, second PIC or 16-bit AT channel. | Core supports several of these for current PC/AT profiles. | The XT descriptor must omit them.  This is a negative construction constraint, not a request to remove their independent Core owners. |
| Product/session surface and corpus | S1 selects a runnable future XT configuration but imports neither firmware nor guest media. | Catalog/YAML routes can carry immutable session requests, but have no XT token or XT-focused tests/fixtures. | Add the XT catalog/YAML request only after the immutable profile exists; tests use synthetic/owner-supplied admissible fixtures and must prove fixed choices/rejections, not silently select PC/AT. |

## Owner Partition

| Layer | Retained responsibility | Prohibited responsibility |
| --- | --- | --- |
| VM profile/resolver | Validate and freeze the selected 5160-268 construction values; copy one plan to Core. | Runtime controller state, IRQ/DRQ mutation, memory/VRAM/media ownership, firmware bytes or timing advancement. |
| Core CPU, board and controllers | Own CPU state, mappings, reset, PIC/DMA/PIT/FDC/HDC/VADP state, deadlines and all guest-visible side effects. | Profile discovery, host presentation state, or mutable session configuration. |
| VM presentation and product | Own host input/presentation and send typed immutable requests through the retained session authority. | A second guest video snapshot, controller state, or CLI-text re-encoding of typed construction values. |
| Firmware/media provider | Supply validated external BYOB resources during construction. | Controller protocol/state, media caches, ROM mapping policy or a hidden XT default image. |

## Exact Source Sweep

- `src/core/machine/cpu_interface.h`: profile enumeration contains no 8088.
- `src/core/machine/cpu_timing_model.c`: source-timing selection has 8086,
  80186, 80286 and 80386 branches, not 8088.
- `src/vm/composition/session/session_interface.h` and session construction:
  no XT profile kind/construction route.
- `src/vm/profile/default_profile/pc_at_profile.c`: current routes and
  descriptors are PC/AT-oriented, including dual PIC and AT controller maps.
- `src/core/machine/controller_interface.h`: HDC personalities contain ATA,
  Compaq WD and IBM WD1003 only; no Xebec protocol.
- T479 S6 remains the Xebec grammar source/cross-model boundary; this audit
  neither changes nor weakens it.

The matrix exhausts the S1 selected BOM.  It intentionally leaves 8088/board
clock and device service-time evidence to S3, and leaves all runtime changes
to the one subsequent functional receiver.
