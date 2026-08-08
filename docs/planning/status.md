# Project Status

## Current Work

**M5 T287 S10 in progress: DOS fixed-disk allocation trace.**

T287 used a lawful, repository-external DOS 6.22 boot floppy and HDD with an
ignored local manifest. The real Console transaction mounted both media and
selected `DEVICE display window`; the separate parameterized checkpoint tool
read only the session's copied presentation mailbox. It reached `A:\>` after
the DOS date/time inputs, observed BDA fixed-disk count `1` and three ATA
`READ SECTORS (20h)` commands, then reproducibly received `Invalid drive
specification` for `C:`. The tool marker is
`M5:T287:S2:WINDOWS31:CHECKPOINT:OK result=c-drive-absent
category=bios-firmware bda_hdd_count=1 ata_commands=3 last_command=20`.

S3--S9 repaired the bounded default-ROM fixed-disk discovery contract: AH=15
now returns type `03h` in AH and the full geometry sector count; AH=08 returns
the parameter-table pointer and user-defined drive type; unsupported calls
return an error; AH=01 observes a clear initial status; and CMOS 12h/19h
declare the matching user-defined type. The owner-built ROM regression carries
the `M5:T287:S9:ROM-INT13-HDD-CMOS:OK` marker. These changes preserve the
existing ATA PIO controller boundary and do not add commands, DMA, display
breadth, CPU behavior, or a Windows support claim.

S11 isolated the fixed-disk parameter-table offset disagreement; S12 corrects
the 16-byte AT table interpretation. The standard sectors-per-track byte is
DPT `+0Eh`; AH=08/AH=15 and the ROM corpus now use that slot, while `+08h`
retains its control-byte value. This corrected contract does not yet change the
external DOS `C:` result.

S13 makes INT 13h/AH=08 return the sectors-per-track value in AL as well as
the CH/CL geometry encoding. The ROM regression asserts AX=`003Fh` for the
fixture geometry. This ABI correction also preserves the external DOS `C:`
failure for further diagnosis.

The external copied-frame checkpoint still reports the same `C:` failure after
S3--S9. Its paused copied-frame observation retains BDA fixed-disk bytes
`00/01/C0/00` for status, drive count, control, and port offset respectively;
DOS has not overwritten the BIOS declaration. S10 records only this
non-invasive allocation evidence before another controller or firmware change
is considered.

## Current Technical Baseline

- **T287 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0287`. The active T287
  subtask has not closed. Artifact `nxvm_0_5_0287.exe` SHA-256:
  `B6612D7C1EFD2864DE777CAA2DB4A3441B7AC00128C61EA29AB6B52B5F321847`.
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
| T287 | Reached the external DOS checkpoint and isolated the first Windows-install prerequisite failure to default-ROM `INT 13h/AH=15h` fixed-disk type reporting; no Windows support claim or artifact. |

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
