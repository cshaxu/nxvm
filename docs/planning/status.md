# Project Status

## Current Work

**Idle.**

M5 T287 S24 repaired the corpus-proven 80386 real-mode address-size defect.
`_kma_linear_logical` now admits actual `SREG_DATA` reads and writes with
32-bit offsets in real mode while retaining the existing code and stack
boundaries. The core corpus proves `REP 67h 66h A5h` copies beyond `FFFFh`
without a fault. The external Setup probe now passes `Reading SETUP.INF...` and
reaches its Welcome page through real DOS, ATA PIO, ROM, keyboard, and copied
presentation. This records one removed CPU blocker only: T287 has not reached
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

- **T287 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0287`. Artifact
  `nxvm_0_5_0287.exe` SHA-256:
  `B35103B1DD1B933D77B39DA7BD134CD8812A42AE2A80CE6CF5C1391BCF03B2E3`.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, bounded `#UD`
  transitions, immutable ROM mapping, and atomic real-mode entry plans.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T280 | Made FDD/HDD candidate replacement atomic; HDD now preserves arbitrary raw byte length through virtual sector capacity, zero tail reads, and padded-tail persistence. |
| T281 | Renamed the sole core controller owners and configuration borrows from historical `shared_*` names to `fdc/hdc`, with no alias or behavior change. |
| T282 | Moved native window/console handle ownership from core to VM platform while preserving copied core input, presentation, and wait contracts. |
| T283 | Extended VM-free controller media evidence, removed unsafe HDD CHS transfer state, and made FDD/HDD persistence collision-safe and failure-preserving. |
| T284 | Froze the first Windows-facing display admission contract for EGA mode 10h and added expected-failing core/VM corpus without changing runtime behavior. |
| T285 | Implemented bounded EGA mode 10h direct, turned the T284 core/VM corpus into normal success coverage, and emitted the 0.5.0285 developer artifact. |
| T286 | Fixed the corpus-proven ATA device-control `nIEN` IRQ14 visibility gap through core-owned controller state, with core, VM-port, and guest fixture success evidence; no DMA, timing, or command expansion. |
| T287 | Fixed bounded ROM CHS device/head, AH=08h caller-pointer, FDISK text-service/window-clear, and HDD-only boot-selection defects; external DOS registers C:, presents stable FDISK copied frames, and hands off from HDD-only ATA boot, while the Standard-mode checkpoint remains a research result, not a Windows support claim. |

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
