# Project Status

## Current Work

**Idle -- M5 Td S40 has admitted the core public-surface closure before the
next Windows CPU task.**

T293--T303 first remove the unused post-`#UD` transition and migrate VM from
raw core configuration/profile borrows to typed core-owned initialization,
controller, firmware, and debug capabilities. T300/T301 then admit separate
pre-decode transition and paused ordinary-RAM transaction contracts; T303
proves a core-only second-consumer fixture without implementing `mantle`.
T304 resumes the Windows Setup 80386 control-transition work at the retained
`MOV CR0,EAX` checkpoint.

T292 remains the latest completed implementation task: the selected
non-overlapping 80386 real-mode forms already conform.

T292 added a prepared-state core corpus for `67h` REPE CMPSB early-stop,
`67h` REPNE SCASB hit-stop, and CS-overridden `67h` REPE CMPSB source selection.
All preserve 32-bit indices/count, FLAGS, memory selection, and fault-free
execution on the 80386 profile. Therefore no source instruction behavior was
changed, no broad 386 claim is made, and no `0.5.0292` artifact is generated.

T291 adds per-slot generation observations and disk-change latches to the
frozen FDC topology. Refresh observes all slots while DIR reports the
DOR-selected slot. DOR loss of readiness cancels an active DMA execution; a
new valid command re-requests DMA. Reset retains its IRQ6/DRQ cancellation.
The core port corpus proves nonselected changes, no-media, motor-off, re-request
and reset; the DOS FDD0 regression remains green. No timing, UI, command, or
topology expansion was added. Full gates passed 120/120 CTests.

T290 replaces the FDC's single media identity with four core-owned frozen drive
slots. A command unit must agree with the DOR-selected drive; ready, media I/O,
and `SENSE DRIVE STATUS` then resolve through that slot. The default PC/AT VM
declares drive 0 only, with the other slots invalid. The focused core port
probe proves distinct drive-0/drive-1 providers and exact selected-absent
`NO_DATA`; the VM binding and DOS FDD0 probes pass. No motor/DIR/media-change
extension, second-drive UI, or command-set expansion was added. The normal
matrix passed 119/119 CTests and the static/governance gates passed.

S1 reproduced the HDD-only Setup checkpoint after file copy and recorded
protected-mode `CALL FAR 0000:00CB` as the internal `#CE(0)` source. S2 added
the minimal 16-bit protected-mode call-gate transfer and matching outer
`RETF` return needed by that trace, with a core prepared-state corpus that
proves the CPL3-to-CPL0 TSS stack transition and return. The external replay
now crosses that instruction and reaches a new, distinct first failure:
`#UD(0)` at `0F 22 C0` (`MOV CR0,EAX`). T288 does not absorb that separate
control-register instruction family. The normal gates passed, including the
new call-gate corpus; T288 publishes its developer artifact and leaves the new
`#UD` checkpoint for a separately admitted task. T289 materializes the
immutable ROM instruction image through the core provider, including the
A20-wrapped reset alias; reset restores only IVT/BDA and the mutable HDD
parameter work area. Its focused corpus and full gates pass.

M5 Td S39 reconciled the documented current source tree with its target
component topology and corrected the T288 evidence boundary. The current
external Setup checkpoint initially completed file copy and stopped at internal
`#CE(0)` while Setup attempted to load Windows. T288 resolved that first
checkpoint and recorded the later `MOV CR0,EAX` `#UD` without retaining any
working-media identity in tracked evidence. Any T288 working-media clone is
task-local and ignored; its path and hash never enter tracked evidence.

M5 Td S38 reordered the evidence-led queue before T288--T292 completed. Td S40
supersedes its future-number allocation: T293--T303 now close the core public
surface, and T304 begins the retained Windows control-transition work.

M5 T287 S24 repaired the corpus-proven 80386 real-mode address-size defect.
`_kma_linear_logical` now admits actual `SREG_DATA` reads and writes with
32-bit offsets in real mode while retaining the existing code and stack
boundaries. The core corpus proves `REP 67h 66h A5h` copies beyond `FFFFh`
without a fault. The earlier external Setup probe passes `Reading SETUP.INF...`
and reaches its Welcome page through real DOS, ATA PIO, ROM, keyboard, and
copied presentation. The later post-copy checkpoint is the current T288
admission. This records one removed CPU blocker only: T287 has not reached
Windows Standard mode and makes no Windows-support claim.

M5 T287 S23 used the repository-external Windows Setup corpus through the real
HDD-only default-ROM and ATA PIO path. A parameterized headless probe reached
`Windows Setup` and `Reading SETUP.INF...`, then reproduced `#GP(0)` at
`0FD7:06BA` with `CR0=0`, `ESI=00042FD8`, `EDI=00110000`, and opcode prefix
`F3 67 66`. The first failure is therefore a real-mode 80386 address/operand
size CPU defect, not a storage, display, or protected-mode transition result.

M5 T287 S22 reproduced the external HDD-only failure before the ATA controller
received any command: the default false `boot_hdd` bit selected an absent FDD.
Composition now retains an `auto` boot preference that selects HDD only when no
FDD is present, while `SET BOOT fdd|hdd` remains a persistent explicit
override. The existing HDD boot smoke now supplies only the external HDD and
proves MBR/VBR handoff through ATA PIO; it also proves both explicit overrides.
No ATA, ROM boot-sector, media backend, guest content, or Windows-support
behavior changed.

M5 T287 S20 reproduced FDISK Options `4` through the retained external corpus,
but its transient-frame assertion was incomplete. S21 traced the subsequent
`INT 10h AH=06h, AL=00h` request for only row 24 and proved that the ROM cleared
the full 80x25 text page instead of the requested inclusive window. The bounded
repair implements valid text-mode `AL=00h` windows with the requested blank
attribute, retains the legacy nonzero-AL behavior, and leaves scrolling
deferred. The owner-built ROM fixture proves a one-cell clear preserves adjacent
text; external FDISK Options `2`, `3`, and `4` now retain their post-Enter
copied frames. No guest media, host shortcut, second video owner, or Windows
support claim enters the repository.

M5 T287 S19 reproduced the external `A:\FDISK` blank display through the real
FDC, ATA PIO, ROM, keyboard, and copied-presentation paths. The program had
cleared B800 text memory, then invoked `INT 10h AH=05h`, `AH=08h`, and `AH=09h`;
the default ROM had silently ignored the latter three services, so no renderer
or VADP defect existed. The bounded repair admits text-mode active page zero,
current-cell read, and repeated character/attribute writes without moving the
cursor. An owner-built ROM fixture proves BDA, B800, and copied-frame results;
the parameterized external probe reports `M5:T287:S19:FDISK:EXTERNAL:OK` only
after the copied frame contains the FDISK partition text. No guest media,
machine-local path, raw trace, host shortcut, or Windows support claim enters
the repository.

M5 T287 S18 repaired the real post-T209 default-ROM compatibility regression:
`INT 13h AH=08h` returned valid geometry but incorrectly overwrote the caller's
`ES:DI` while locating the DPT. DOS consequently wrote its fixed-drive BDS
geometry to the wrong location and divided by zero before registering `C:`.
The ROM now preserves the caller pointer. The external FDD/HDD checkpoint
reaches the date/time prompts, `A:\>`, and `C:\>` through the real FDC, ATA
PIO, ROM, IRQ, DMA/PIO, and copied-frame paths. The ROM fixture also locks
the caller-pointer boundary and direction-flag-independent PIO transfers.

The reported EDIT colors were not a regression: both T209 and current use
attribute `14h` (blue background, red foreground) for the observed text.
The guest text memory, VADP copied frame, and Win32 GDI pixel output agree.
S18 adds a GDI palette probe without changing the retained display-window
foreground and focus behavior. It does not add a renderer-side VRAM copy,
host media shortcut, CPU admission, or Windows support claim.

T287 used lawful, repository-external DOS and Windows-install media through
the real FDC, ATA PIO, ROM, IRQ, DMA/PIO, and copied-frame paths. No guest
media, binary, machine-local path, or raw trace is tracked. The real Console
observation selected `DEVICE display window`; the headless checkpoint consumes
only the copied presentation mailbox.

Earlier T287 work corrected the bounded default-ROM fixed-disk discovery
contract, including `AH=15h`, `AH=08h`, DPT sectors-per-track placement,
status handling, and CMOS Type 47 encoding. The closing work found and fixed a
separate ROM defect in `AH=02h` and `AH=03h`: after reusing `DX` for an ATA
port, the ROM had formed device/head from the port high byte rather than the
caller's `DH`. It now preserves the caller head and writes `A0h | head`.
The owner-built, parameterized admission probe enters through real `INT 13h`,
uses nonzero `ES:BX`, byte-compares the MBR and VBR against the external image,
and reports `M5:T287:S16:HDD-ADMISSION:OK`.
The retained ROM regression also writes CHS `0/0/1` through `AH=03h` to a
temporary HDD and reads its `5AA5h` word back through `AH=02h`.

The retained ATA PIO DOS smoke deliberately zeroes its temporary HDD backing
before boot. It continues to prove ATA PIO through the guest path without
coupling the controller regression to external bootable-media state.

## Current Technical Baseline

- **T291 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0291`. Artifact
  `nxvm_0_5_0291.exe` SHA-256:
  `9CD9912B27D9B3FB199480788E5E21E6A490904C9C4C751FE31D2224961E878A`.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, the current
  post-`#UD` transition pending T293 removal, immutable ROM mapping, and
  validated real-mode entry plans.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T285 | Implemented bounded EGA mode 10h direct, turned the T284 core/VM corpus into normal success coverage, and emitted the 0.5.0285 developer artifact. |
| T286 | Fixed the corpus-proven ATA device-control `nIEN` IRQ14 visibility gap through core-owned controller state, with core, VM-port, and guest fixture success evidence; no DMA, timing, or command expansion. |
| T287 | Fixed bounded ROM CHS device/head, AH=08h caller-pointer, FDISK text-service/window-clear, and HDD-only boot-selection defects; external DOS registers C:, presents stable FDISK copied frames, and hands off from HDD-only ATA boot, while the Standard-mode checkpoint remains a research result, not a Windows support claim. |
| T288 | Resolved the external Setup post-copy `#CE` with the bounded 16-bit protected-mode call-gate and outer-`RETF` path; core and real replay now reach the next `MOV CR0,EAX` `#UD` checkpoint without claiming Windows support. |
| T289 | Materialized the default PC/AT ROM image before provider freeze, including its A20 reset alias, while reset restores only IVT/BDA and mutable device tables; ROM, boot, Console, debugger, and display regressions pass. |
| T290 | Replaced the FDC single-media binding with frozen drive slots and exact DOR/unit selection, proven through core ports, VM composition, and DOS FDD0 regression without broadening FDC behavior. |
| T291 | Made FDC generation, disk-change and motor/DMA cancellation per frozen drive slot, with core port and DOS FDD0 evidence while leaving timing, UI, and commands deferred. |
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |

Detailed contracts, commands, artifact provenance, and prior closures are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S30:** compacted completed M5 planning records and formalized the
  then-current T264--T278 forward queue.
- **M5 Td S31:** made `status.md` the sole current-baseline authority, retired
  completed T264--T266 records, and added a documentation-governance gate.
  This task changes no runtime behavior or active-task scope.
- **M5 Td S32:** admitted the original second core/composition migration.
- **M5 Td S33:** corrected it using the external-consumer boundary evidence:
  T271 admits only opaque policy-free host capabilities in `core/platform`,
  extends the migration through T278, and shifts the former forward queue to
  T279--T287. Filesystem, path, mount, and DOS policy remain above core.
- **M5 Td S34:** repaired the historical queue wording, inserted the approved
  T279--T283 reliability/interface package, and shifted the older future queue
  to T284--T292. This task changes no runtime behavior.
- **M5 Td S35:** tightened T279 bounded-format semantics, T280 raw-HDD commit
  truth, T282's existing-consumer-only host-surface audit, and T283's direct
  dependencies. This task changes no runtime behavior.
- **M5 Td S36:** recorded the WASM media-backing admission boundary without a
  runtime, API, artifact, or task-number change.
- **M5 Td S37:** made M5 history static, capped the status closure summary,
  and hardened the mojibake governance check. This task changes no runtime
  behavior or active-task scope.
- **M5 Td S40:** inserted T293--T303 core public-surface closure, included
  RTC/CMOS/NMI and opaque firmware capability in the migration boundary, and
  shifted the remaining Windows CPU/FPU/readiness/closure work to T304--T308.
  This task changes no runtime behavior or artifact.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Project Status](status.md) and [M5 convergence queue](m5-pcat-hardware-convergence.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
