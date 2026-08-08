# Project Status

## Current Work

**Idle.**

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
S18 adds a non-activating GDI palette probe and removes automatic foreground
and focus acquisition by a newly shown display window. It does not add a
renderer-side VRAM copy, host media shortcut, CPU admission, or Windows
support claim.

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
  `226F4BCB4AF842923C3A490307E72629FBAB7C8A56E1D0A2DF5987BF582E7DE3`.
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
| T287 | Fixed bounded ROM CHS device/head and AH=08h caller-pointer defects; external DOS now registers C: while the Standard-mode checkpoint remains a research result, not a Windows support claim. |

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
