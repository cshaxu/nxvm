# M7 T44 S2 Mapper Qualification

## Approval And Reference

Owner accepted S1, reviewed the design audit, then approved implementation
on 2026-10-06. Baseline ac5861d90 / S1 production b2c973f69, current 0044 pair.
Changes are MyNES-only; no Shared source/test/configuration, NXVM, sibling,
owner INI, ROM or saved snapshot change is included.

Before editing, S1 probes were retained under ignored build/t44-s2-reference:
x64 SHA-256 1FBDA65ECB213F7E85AC032E3CD9D882364BB46174A13F2971E5E6C147958766;
x86 SHA-256 7EC16687E0DB5C55D083A546FE2D5B3A8095E79B69EE07AC84867B1120F960A4.
Both versions use the same benchmark source and optimized/static build flags
from S1. No pre-palette executable is used as S2's reference.

## Mechanism And Disposition

- NROM: original direct/mirror path retained.
- MMC1: for the already admitted two PRG banks/eight CHR pages, remove repeated
  variable division/modulo, preserving every control/bank bit interpretation.
  No new persistent MMC1 state or widened board acceptance.
- UxROM: use the admitted eight-bank domain in reads; keep the ROM bus-conflict
  write path and wrapping of raw restored bank bytes unchanged.
- CNROM: no performance cache or modulo-to-mask conversion. Fix only restored
  bank bounds, before live state publication.
- MMC3: one owner-local builder computes four 8 KiB PRG and eight 1 KiB CHR
  offsets at construction, bank-select/data writes and validated snapshot
  restoration. Reads use the selected offset plus window-local address.
  Runtime modulo remains in the builder for non-power-of-two capacities.
  The old per-read bank-selection formula is retired, not kept as a fallback.

The arrays contain 48 bytes of private offset data; no new allocation, registry,
thread, dirty flag, generation, public interface or decoded tile cache.
Registers are authoritative; offset fields are not serialized in snapshot v3.
Snapshot staging reconstructs them only after all input/staged-memory checks,
before publishing the candidate. Rejected snapshots leave live state unchanged.
Reset does not change cartridge registers and therefore needs no new reset path.

PPU A12 calls, IRQ state transitions, bus events, guest scheduling, CPU/APU/PPU
algorithms and S1 frame conversion are unchanged. The private CHR address helper
remains shared by ROM reads and mapped CHR-RAM writes.

## Complete Mapping And Snapshot Proof

New code-owned mapper_mapping_smoke covers:

- Every accepted MMC3 PRG size (31 counts, 2-32 header banks) and CHR size
  (33 counts, 0-32 header banks including RAM) as independent axes: 64 fixtures,
  not 1023 duplicate combined images. Each axis includes non-power-of-two sizes.
- All eight bank-data registers, every byte value and four PRG/CHR mode
  combinations; all 256 bank-select bytes, including unused-bit aliases.
- Start/end of every CPU/CHR window against independent original formulas,
  both exact derived offsets and actual bytes; mapped CHR-RAM writes.
- All 32 MMC1 control values crossed with all 32 bank values using production
  serial writes, both PRG/CHR windows; all 256 raw UxROM restored bank bytes.
  Existing mapper tests retain bus-conflict, mirroring and serial semantics.

Expanded snapshot_smoke covers changed-bank MMC3 restore for CHR ROM and RAM,
every CPU/CHR address after restoration, preserved mapping across warm reset,
and byte-identical v3 reserialization. CNROM capacities 1-4 restore valid banks
and reject every out-of-range byte while comparing live snapshots before/after.
All malformed input is generated in test code; no external files in new units.
Existing MNS1 v3 size/version and continuation assertions remain intact.

Full MyNES suites run once per width for final source:
x64 57/57 in 54.01 seconds; x86 57/57 in 73.54 seconds.
Each includes 45 unit/static and 12 integration cases, including native Window,
native Console, lifecycle, input, reset, debug, snapshots and mapper IRQ timing.
Strict warning builds pass. Focused mapping/snapshot checks were diagnostic
before that single final qualification, not replacements for it.

## Performance Evidence And Limits

Reuse S1 warmup/input/frame/quanta/hash contracts. All six ROMs, both output
modes and both widths preserve ROM identity, cycles, instructions, frames,
PCM and serialized-state fingerprints. No trap or dropped PCM; graphics/text
guest identities agree. Guest work is never skipped to improve host timing.

Initial whole-matrix wall-clock measures on processor zero were noisy, including
large changes in untouched NROM. Adjacent variants per game on processor two
still show wall-clock variance. They are retained as diagnostics, not precise
universal speed percentages. No native displayed FPS or per-device attribution.

The MyNES-owned tool now also records each leaf's TotalProcessorTime after exit.
This excludes descheduled time but includes construction, all five batches'
warmup (300 frames), measured guest work (600 frames) and checksum/printing work.
Its single process total repeats in CSV rows: take it once, never sum five times.
It is not a pure Core timer or per-frame latency; CPU frequency/load can still vary.
Workloads/Modes selectors enable adjacent bounded pairs, preserve the full default
matrix and canonical graphics-before-text validation order.

Whole-probe CPU consumption in milliseconds, same processor and fixed workload:

| Width | Workload | Before CPU ms | After CPU ms | Observed reduction |
| --- | --- | ---: | ---: | ---: |
| x64 | smb1 | 3296.875 | 3031.250 | 8.06% |
| x64 | drmario | 3671.875 | 3406.250 | 7.23% |
| x64 | jackal | 2921.875 | 2875.000 | 1.60% |
| x64 | smb2 | 3156.250 | 2890.625 | 8.42% |
| x64 | smb3 | 3500.000 | 3187.500 | 8.93% |
| x64 | tmnt3 | 4203.125 | 4015.625 | 4.46% |
| x86 | smb1 | 3187.500 | 3187.500 | 0.00% |
| x86 | drmario | 3859.375 | 3828.125 | 0.81% |
| x86 | jackal | 3015.625 | 3062.500 | -1.55% |
| x86 | smb2 | 3421.875 | 3218.750 | 5.94% |
| x86 | smb3 | 3687.500 | 3703.125 | -0.42% |
| x86 | tmnt3 | 4484.375 | 4187.500 | 6.62% |

Totals: x64 20750 -> 19406.25 ms, observed 6.48% reduction;
x86 21656.25 -> 21187.50 ms, observed 2.16% reduction.
Do not attribute the NROM control's variation to bank optimization. Small
per-route changes are not claimed as significant; x86 Jackal and SMB3 have
small increases. The clearest cross-width MMC3 gains are SMB2/TMNT3; blanket
per-game or real-time FPS promises are rejected. Together with repeated removal
of per-read division/selection, bounded code/state and exact semantic proof,
the candidate is retained; there is no materially reproducible regression claim
hidden by replacing a slow route with a compatibility path.

Ignored results: build/t44-s2-paired-<width>-<game>-<variant> retains both modes;
build/t44-s2-cpu-<width>-<game>-<variant> retains the CPU-total graphics samples.
Reference and candidate run sequentially with ProcessorIndex 2 and Workloads
selecting one game. The CPU-total comparison uses Modes graphics; correctness
qualification still covers both modes. No build/test competes with measured pairs.
Each inspected probe is a childless leaf, bounded to 60 seconds, waited/killed
on timeout and disposed. No physical endpoint or manual sound observation needed.

## Quality, Scope And Delivery

Similar-issue sweep examines all MyNES mapper reads/writes, creation and
snapshot commit paths, and all bank-state assignments. Only cartridge register
writes and the snapshot reader mutate mapping state in production. Constructor
and both mutation paths invoke the one builder. UxROM wrapping and MMC1 bounds
remain safe; CNROM is the sole unchecked restored mapping with an allocation
overflow path and is now rejected. MMC3 arbitrary bank bytes are safe through
actual-capacity modulo. Source searches include mmc3 offsets/rebuild_mapping
and bank-register assignments; PPU A12/read call sites stay byte-unchanged.

Production diff: cartridge.c +38/-42, cartridge.h +5/-0, snapshot.c +3/-1:
+46/-43, net +3 lines. No extra layers; cartridge.c itself is four lines shorter.
Count the new unit and expanded snapshot regressions separately at delivery.
Derived state is not a second authority or a persistent media cache.
Counted source/test paths are cartridge.c/h, snapshot.c, unit/core/CMakeLists.txt,
unit/core/mapper_mapping_smoke.c and unit/core/snapshot_smoke.c beneath MyNES.
Staged git diff --numstat gives +264/-43, net +221, of which +218 lines are
test registration and code-owned matrix/failure-atomicity proof. The measurement
tool adds ten/removes three lines (net +7); documents and binaries are excluded.

Both current 0044 products are rebuilt, optimized and stripped (no .debug):
x64 PE=8664, 249358 bytes,
SHA-256 12E7686B0DF97E4C0954032AC7BC5E60C3D06B22C5A7618045FF97FA4D959A68;
x86 PE=014C, 276494 bytes,
SHA-256 418B1B161938E2FCAC4D957D0AD44C811549DB0674DF5D0F6438AEDA2F7A9357.
Same task revision, new source/hash identity; no additional old EXEs retained.
INI SHA-256 remains
198846D0D4EB3C7CAB1357992D7AB3441BECA676706AF1BAB0DAF507FB6BCC15.
Owner snapshot bytes and external masters remain unchanged.

Documentation governance, diff checks and unchanged Shared/NXVM scope checks
pass. No Shared manifest rebuild or receiving NXVM artifact change is needed.
Warm caches and bounded reference/results remain for immediate task closure
review; original processor-zero trials may be removed after exit as nonaccepted
timing evidence. Actual-diff coordinator review and pushed delivery are required
before S2 closure. T44's S3 task-level reconciliation is not claimed here.

## Coordinator Acceptance

MyNES P1 b22a3bb89 was committed and immediately pushed to origin/master.
Actual immutable-diff review confirms scope, one-owner mapping, all rebuild
boundaries, modulo/mask domains, failure atomicity, test coverage, performance
limits and paired artifact identities. S2 is accepted; the subsequent pure
governance P closes it. T44 remains open, and no S3 is automatically admitted.
