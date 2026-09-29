# T539 S14: HDC Receiver Verification

Source baseline: b2cbaf74c; Shared implementation is 9020d8bba and the
NXVM receiver/artifacts are delivered by the S14 P2 containing this record.
The [boundary record](../architecture/t539-s14-hdc-extraction.md) records
ownership, original-case migration and semantic corrections. This record
does not itself assert coordinator acceptance; Current and task history
record the subsequent actual-commit review.

## Verification

- Complete units: 358/358 per width; x64 202.83 seconds, x86 38.94 seconds.
- Final default external integrations after diagnostic-fixture cleanup:
  20/20 per width; x64 22.28 seconds, x86 22.41 seconds.
- Each remaining profile/width boot ran once. XT x64/x86: 18.70/24.39
  seconds; AT: 37.60/44.02 seconds; Model 40: 61.82/79.36 seconds.
  These probes use the actual selected INI, external media and embedded
  firmware objects, and prove the named boot checkpoint, not all manual UX.
- Standalone tools-off x86 package: 25/25, including 21 runtime cases
  and four corpus/manifest checks.
- Specialized aggregate, six manifests, documentation and whitespace gates
  pass. Six manifests were rechecked after the final comment-only cleanup.
- All 66 construction rollback cases pass on both widths. They cover chip
  allocation and every HDC port registration position for all personalities,
  preserve existing FDC routing, leave no Xebec DMA request and allow retry.
- Final code review corrected the obsolete Xebec DCB comment and documented
  existing optional callback behavior. No production executable logic changed
  after these full runtime runs. All eight products were rebuilt after the comments.

## Receiving Products

Eight optimized stripped 0.5.0539 products are deployed under
`assets/nxvm/<profile>/`. Their build-time PE checks pass; direct header
inspection confirms 8664 for x64 and 014C for x86. Both reused build trees
finish configured for default. Keep the three bounded build/t539-s3 trees
for final acceptance and the next chip batch.

MyNES's product/core CMake graph links Common, Types, Base, Storage and Audio,
not x86 chip targets. Neither its source nor its 0043 pair changes. Git
comparison confirms no tracked owner INI changes. External originals remain
external; the approved product EXEs retain their embedded ROMs.

| File | Bytes | PE | SHA-256 |
| --- | ---: | --- | --- |
| nxvm_model40_0_5_0539_x64.exe | 1310893 | 8664 | F3C23E3D999F93173EBC41DB46FBFFDA4B6215EAA63AB95F52D0E03506AE6AEC |
| nxvm_model40_0_5_0539_x86.exe | 1451344 | 014C | 818EFDEE11F64DA02FA0C3588A06A1D385868F3FC8D209C186358A64A64C5BE0 |
| nxvm_default_0_5_0539_x64.exe | 1327210 | 8664 | B9CD895151B467EBDD98157EFA07FA26E5B11640E9E711755A495F67D95DBAFE |
| nxvm_default_0_5_0539_x86.exe | 1467659 | 014C | 7C939F6D6E9F71411E473E97544FAC76F4C7994451372902968DBC28D291499D |
| nxvm_xt_0_5_0539_x64.exe | 1327178 | 8664 | 4E540237E8DD650497FCCCDB1B88DD044D7E0F7910E1F55546B19BF74351424B |
| nxvm_xt_0_5_0539_x86.exe | 1467626 | 014C | 5F373B8E7DC233210DEAFD3F2EB155B4C56A5BE06D12EF4563CFD2A53EFBFB3D |
| nxvm_at_0_5_0539_x64.exe | 1327244 | 8664 | 966DCB2A657A7D934234E85FC77D32F23087B35FF9F94F741743D3E5EFCEE19C |
| nxvm_at_0_5_0539_x86.exe | 1467694 | 014C | 97E4F884238E09F97AA29BA6A685F074DD8DF1C2F375D6EBD13935F7F8AD6F7B |

## Actual-Diff Review And Case Ownership

| Requirement | Implementation and direct proof |
| --- | --- |
| One independent controller owner | `x86-hdc` links only Types. Its opaque instance owns task-file/DCB, command capture, one sector buffer and timing. Independent tools-off tests and the explicit corpus DAG reject App/private dependencies. |
| Real board adapter, no second engine | App hdc maps port selectors, media IDs/results/byte offsets and signal bindings. The scheduler supplies absolute due time. No App data/xebec field access or old immediate/elapsed advance remains. |
| Original ATA semantics | Shared `hdc_taskfile_smoke.c` retains the original reset/NIEN/status, media error, no-pre-read, captured-command, seven/three-tick, two-direction multi-sector, IDENTIFY and pending-reset assertions. App removes 160 pure timing/phase lines and retains PC/media registry/WD clock proof. |
| Personality preservation | Shared contract cases own the former Compaq command block and explicit WD differences. App Compaq retains master/slave IDs, absent drive, 3F7 wired-OR, IRQ and ports. Xebec wiring retains DCB/sense/initialize plus real DMA channel/PIC/RAM/media transfer cases. No ESDI or general ATA substitution. |
| Due-now and reset epochs | Independent cases cover all four protocols, tick-zero command/sector work, no deadline while waiting for data, repeated timestamps, backward-time rejection, cold epoch and guest SRST preservation. |
| Output lifetime and rollback | Active Xebec reset/destruction withdraw DRQ/IRQ. Owner-local allocation failures and 66 board registration/allocation rollback cases prove no published instance/new route and successful retry. |
| Media address/capacity | One u64 sector index replaces host-size byte-offset arithmetic; large-LBA read/write cases run on both widths. Xebec zero-capacity and wrong record-size cases preserve the old registry rejection and do not call transfer providers. |
| Diagnostic ownership | Boot/debug probes consume copied observations, never chip layout. Terminal Windows diagnostic readers first confirm pause and join the worker before sampling HDC/CPU; a copied struct is not concurrent publication. |
| Receiving artifacts and assets | Unit/default integration/vendor checkpoints and eight PE/hash identities above; MyNES has no changed link input; no owner INI or external original changed. |

The review separates mechanical relocation/name substitutions from actual
changes: phase-based due-zero admission, absolute scheduling and SRST epoch,
output withdrawal, u64 sector arithmetic, restored Xebec capacity validation,
checked construction and one rollback exit. Command tables and personality
restrictions are preserved. Copied observations do not become a production
scheduling interface. No chip thread, host file, wall clock or generic device
framework is added. Existing timing grades and incomplete silicon coverage
are unchanged.

The diagnostic follow-up reproduced a test-owned error: stopping a running
Core can cold-reset its HDC counters. A join alone therefore removed the
evidence being inspected (the checkpoint failed with zero commands even after
DIR output). The final fixture waits for the published DIR file summary,
confirms Common pause, then joins and reads the stable observation. It keeps
the nonzero ATA-command predicate and removes the racy command-count polling.
The pause diagnostic observed 13 commands and passed; its temporary print is
removed. Setup's terminal report follows the same pause/join boundary. These
are test-only edits; no lifecycle or HDC production behavior is changed.
The final complete default integration suites pass on both widths. The optional
Windows setup diagnostic also compiles on both widths; this is build evidence,
not a claim that its separate runtime scenario was executed.

## Code Size And Simplicity

Counted with staged Git numstat against b2cbaf74c over changed src/test/cmake
paths, including new files and excluding manifests, documentation and EXEs:

- Seven C/header source files: +1502/-1198, net +304.
- Eighteen C/header test files: +1039/-367, net +672.
- Five build/static-check files: +31/-15, net +16.
- Total: +2572/-1580, net +992 lines.

The positive source delta pays for the opaque public value/callback boundary,
checked allocation/lifetime and real PC/media adapter; the old embedded-state
engine is removed, not retained behind a facade. The larger test delta adds
independent code-owned media fixtures, four-personality time/lifetime cases,
large-LBA and capacity failures, and exhaustive construction rollback. The
complete ATA assertion sequence is independently reusable while board-specific
media/port checks stay local. Three duplicate constructor failure exits become
one. No production cache, copied live state or second command engine remains.

CPU/FPU, video and the remaining finite inventory are still T539 work;
successful HDC receiver verification does not close the whole task.
