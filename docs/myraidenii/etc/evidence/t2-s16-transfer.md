# T2 S16 Ordinary Native Transfers And Addresses

Baseline `1c48b9d63`. This consumes V06's ordinary native MOV, XCH,
LDEA/LEA and TRANS/XLAT result/address/data-order subfamily, not complete
V06/native slots, PSW/control/entry, mode/8080, prefix/bus timing, Machine
or board/game qualification. S14/S15 remain the stack/segment owners.

## Primary Source And Existing Production Path

The pinned [NEC October 1986 manual](../research/references.md), SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
printed 12-4--12-13 specifies native register/immediate/memory/accumulator
MOV, exchange, effective-address load and byte translation. These operations
preserve flags. Word/address arithmetic is native sixteen-bit; the separate
S8 physically contiguous data-word tail retains its explicit L2 source grade.
No external implementation, table, ROM or manual payload is imported.

The first actual execution/preview matrix passes the existing source. No
production correction, refactor, helper, interface, type/layout, state or
timing assumption is added merely to make a migration diff. All operations
remain in the original single executor, decoder and scalar operand owners.

Source audit query:

```text
rg -n '^static void (MOV_|XCHG_|LEA_|XLAT)|static void _m_read_rmw|static void _m_write_rm|static void INS_C[67]' src/x86-core/cpu/cpu_instructions.c
```

- MOV 88--8B and immediate C6/C7 groups use the existing ModRM/operand routes.
  B0--BF register immediates and A0--A3 moffs retain their genuine encodings.
  The register-width mask/write owner preserves unrelated bytes/high halves.
- XCHG 86/87 and the 90--97 accumulator encodings retain original snapshot and
  register-write ordering. `_m_read_rmw` prepares the modifying operand before
  data reads; `_m_write_rm` uses the original write path. Tests validate actual
  reads followed by writes and CPU rollback with accepted memory prefixes.
- LEA uses the decoded offset without a data transfer. XLAT wraps BX+AL as a
  sixteen-bit offset in the selected source segment, retaining the lookup
  byte as its only data read. Neither is replaced by a test-generated result.
- Genuine Intel 32-bit/protected alternatives and MOV control/debug/test
  registers are not native V30 forms; they are unchanged. S14/S15 retain the
  already accepted native stack/segment behavior. S12 reserved-form admission
  and S13 instruction-fetch/preview boundary tests remain enabled.
- `_m_read_rmw` currently admits the physical memory-lock callback only for
  486. Native BUSLOCK, physical arbitration/interleaving and prefetch clocks
  remain explicit V05/V14/V15 receivers. Sequential native XCH read/write
  results are not evidence of electrical lock behavior. No native lock model
  is silently invented or certified by this batch.

## Finite Independent Proof

`cpu_v30_transfer_smoke.c` reuses the original one-MiB real-storage fixture
unchanged. An input operation describes the expected architectural assignment;
production decoding/metadata and another CPU implementation are not its oracle.
Separate arithmetic computes the effective address from named input seeds and
encoded displacements. CPU result comparison includes all GPR bytes/high halves,
segments, flags and instruction pointer, not only a convenient destination.

The 55,200 success vectors, each with observational preview and actual CPU
execution, comprise:

- 17,280 memory and 3,840 register cases across MOV 88/89/8A/8B and XCH 86/87:
  every ModRM, both widths, five segment choices and three memory/two register
  seed contexts. Base/index sums, signed short/word displacements, BP's default
  SS, explicit overrides and sixteen-bit address wrap are independent inputs.
- 2,880 legal memory-form LEA cases across all ModRM/default/override/seed
  combinations; no data read or write is permitted.
- 80 accumulator exchange/NOP-alias cases with both flags contexts.
- 20,480 byte-register immediate cases (all 256 values, every low/high byte,
  both B0 and C6 encodings, all short segment selections), plus 480 word-register
  cases with six boundary/distinctive values and both B8/C7 encodings.
- 4,320 immediate-memory cases across every legal /0 memory form, both widths,
  segment choices, seeds and six values.
- 720 moffs cases at zero, one, aligned/odd ordinary addresses and FFFE/FFFF,
  with both directions/widths, source segments and six operand values.
- 5,120 XLAT cases covering all 256 byte indices, four table bases and five
  segment selections, including BX+AL wrap and code/data overlap under CS.
  The final input byte after code placement is the actual lookup stimulus,
  not a fabricated constant output or a future self-modifying-code claim.

Every applicable data-provider read and write position is refused in turn.
Failures must stop with complete CPU rollback, no invented guest exception,
no shadow publication and only already accepted write bytes. Actual data
records check address, width, direction, accepted ordinal and each paired
BEGIN/COMMIT/CANCEL phase. Exchanges must finish reads before writes. Pure
preview preserves the CPU and performs no data/phase effects; observational
code accesses remain distinct from actual data transfers.

Memory-effect proof uses the fixture's audited sole read/write callbacks:
reads only copy from storage; each accepted write changes exactly its recorded
address/width. The checker enumerates every such record, rejects any additional
or wrong-address access and checks operand bytes, including an adjacent guard
for byte operations, against the accepted prefix. No whole-RAM copy is needed for zero-write register,
LEA or load cases, and no write can bypass that fixture callback in the inspected
CPU owner. This removes repeated bulk copying, not an existing assertion or
production validation. Existing S8/S14/S15 full-storage witnesses remain intact.

## Actual Delta And Verification Boundary

Production +0/-0/net 0. Tests/registration +332/-0/net +332, excluding
manifests/documents/artifacts. The actual increase is below the initial
+400--600 estimate because one real-storage fixture, twelve local verification/input
helpers and one finite operand model serve every admitted form without
duplicated fixtures or RAM-copy scaffolding. Source formatting is not compressed
to meet a line target. No public API or production path is introduced.
Existing production warning exemptions remain unchanged.

The first x64 target run passes in about 2.13 seconds. Frozen test SHA-256:
`0CA4FB074C5F305079AF9D75EB6303775FF1366329C4909B87FA981F241305A6`.

Both original Release Ninja/ccache graphs build all targets. First complete
repo-only suites pass x64 668/668 in 78.88 seconds and x86 668/668 in 86.44
seconds, including 179 X86 Core units and all receiving App units. Architectures
run sequentially; no retry, budget change or assertion removal is needed.
The new unit passes in 2.02/2.01 seconds under its unchanged thirty-second
budget. All registered manifest, Types, inward-dependency, ownership and
negative gates pass; separate manifest/corpus and documentation governance
checks pass too.

Production source remains byte-identical to S15, including instruction SHA-256
`CC8FD2C29CFD14306340AC5A10FE71FD3A0E61DD891C7315947148750F9BD8AB`.
Test registration is not an executable input. Fresh checks of all eight PE
signatures/widths, product names, revision 0563, lowercase INI identities,
sizes and hashes match the [S15 artifact inventory](t2-s15-pc-artifacts.json)
exactly after both all-target builds. No affected production input changed,
so the execution rule's test-only determination requires no artificial EXE
relink/deployment or binary commit. MyNES pair and all INIs/media/ROMs remain
unchanged; original graphs/cache and default profile are retained.

Coordinator accepted Shared `7c00ae072` and evidence `00368b9c9` after actual
unit/helper/matrix, registration/manifest, original source-owner, complete
verification, retained artifact and document review. The sixteen-field packet,
original full goal, complete admitted batch, unchanged production/ten EXEs,
scope/P sequence and remaining receivers were reconciled. No original domain
member is silently retired. S16 closes through the following purely governance
P; no extra code/artifact change is introduced. Logs under
`build/myraidenii/t2-s16-*` remain needed for final T2 qualification;
both build/test handles have exited. No external-ROM integration, manual
desktop/RDP, physical audio, board or game execution is claimed. No MyRaidenII
EXE exists; the lossless-colour [High TODO](../../states/TODO.md) remains a
final-release gate. All original native/mode/entry/time/Machine/board/game
requirements remain open; this result does not close T2 or the full goal.
