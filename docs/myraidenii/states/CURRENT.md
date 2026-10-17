# MyRaidenII Status

## Current Work

M1 T1 S2 is active after accepted S1. This batch makes the ROM inventory audit
repeatable, finishes source inspection and records concrete CPU/board receiving
contracts. It does not implement the CPU or change Lib.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T1 S2 after accepted S1 P2 c34b5b2ad; P starts at P1. Single-session coordinator/executor. |
| Admission And Approval | Standing owner objective authorizes MyRaidenII implementation under governance. Owner supplied the external ZIP and approved future V30 profile/NEC-space/timing-origin enum expansion and CPU integration; Lib change remains expressly declined. S2 target is MyRaidenII only, no Shared or peer runtime edits. |
| Objective | Deliver a repeatable read-only inventory verifier with ROM-free negative tests, complete primary sound-source inspection, and freeze concrete CPU/board/qualification receiving contracts for T1. |
| Non-goals | No Shared runtime or API edit, CPU/chip implementation, game execution claim, ZIP extraction/download, ROM redistribution, Lib frame change or product EXE. |
| Reference Baseline | Clean c34b5b2ad; accepted S1 inventory/audit/ledger and pinned originals. Owner-provided ZIP is revalidated, never modified. |
| Candidate Proposal | ../proposals/m1-board-and-source-freeze.md; ../history/M1-T1-board-and-source-freeze.md; consume remaining F06/F08/F09/F13 freeze rows and strengthen F01--F04 reproducibility. |
| Files And ABI Surface | tools/myraidenii/Verify-RomInventory.ps1, product-owned test/myraidenii CMake/ROM-free unit script and README, and docs/myraidenii. Approximate new tool/test code 300--400 lines. No C ABI, Shared manifest, root build, INI or binary input change. |
| Applicable Rules | Execution complete P, separate coordinator review and per-target commits; Architecture single immutable audited inventory/no alternate runtime loader; Coding bounded streams/no extraction or generic archive framework; Documentation/source policy. Apply architecture-governance then coding-governance; PDF skill for scanned original tables. |
| Verification | Run the actual verifier on owner ZIP and independent synthetic success/failure cases through both native Windows PowerShell host widths; register and run the product tool unit in standalone CTest. Run existing full repo-only x64/x86 unit suites in reusable graphs. Pin/inspect source documents, validate current packet/queue/links, documentation gate, actual diff and unchanged existing executable hashes. No emulator runtime result follows from metadata/tool tests. |
| Expected Markers | MYRAIDENII:ROM-INVENTORY:OK for correct archive only; synthetic corrupt/missing/duplicate/path/hash/container cases fail with the intended diagnostics; source and boundary ledger distinguishes frozen inputs from unimplemented runtime. |
| Asset Needs | External owner-supplied original US-set-1 ZIP; synthetic bytes for units; original NEC/Zilog/Yamaha/OKI and Seibu references. No protected payload enters tracked files. |
| Reporting Requirements | Scope/estimate, targeted verification and source findings, actual tool/test line delta, unchanged Lib/Apps/artifacts, remaining owner-directed full-colour gate and complete-P review outcome. |
| Stop Conditions | Shared/API editing under S2; code/license import; protected payload publication; unresolved evidence used to assert exact hardware behavior. Keep Lib TODO intact and continue independent source/tool work. |
| Exit Criteria | Complete tool with bounded read-only behavior, meaningful independent negative coverage, actual ZIP proof, source/contract/checkpoint records, applicable gates and actual-change review, committed/pushed P. T1 closes only after its complete freeze ledger is reconciled; no claim that CPU/gameplay is implemented. |
| Original Owner Request | Implement MyRaidenII in single-person dual-role mode under governance, with owner input ZIP and no Lib modification. |
| Similar-Issue Sweep | Archive role/length/container distinction, duplicate/path/oversize/hash failure admission, stale source statuses and mismatched task state; inventory metadata is the only audit authority, not a second runtime manifest or loader. |

| Task progress | Accepted result / remaining work |
| --- | --- |
| T1 S1 | Source/set/contract audit accepted. All 13 original-US-set-1 runtime ROM identities verified, primary references pinned and CPU/Machine/host reuse risks recorded. T1 remains open; OKI source inspection, final boundary review and checkpoint freeze remain. |

## Accepted Evidence And Review

The complete S1 packet and delivery are retained in `972fc7715`.
[Audit](../etc/evidence/t1-s1-source-and-contract-audit.md),
[inventory](../etc/evidence/t1-s1-rom-inventory.json) and
[freeze ledger](../etc/t1-freeze-ledger.md) distinguish data/source qualification
from unimplemented runtime behavior. Review inspected the actual 11-file
diff (+530/-31 documentation/metadata lines; source/test +0/-0), all 14 ZIP
members, the independent 13-image reference comparison, 16 packet fields,
six ordered Queue candidates and 46 relative links. Documentation/whitespace
checks pass; all ten existing EXE hashes remain unchanged.

The owner requires no Lib change. Its 256-colour graphics contract versus the
board's 2048-entry palette is recorded as [High TODO](TODO.md), not silently
quantized, bypassed or declared compatible. CPU/board/source work may proceed;
full-colour release remains gated by that unresolved contract.

The source and assets roots retain descriptive README files; the test root now
registers the real inventory-tool unit. There is no
MyRaidenII EXE, V30 implementation or successful game execution to report.
Research-only work does not rebuild unchanged executable inputs; no runtime,
desktop/RDP or physical-audio qualification is claimed.

## Previous Milestone

M0 Td S1 is owner-accepted and closed at `f21a22997`. Bootstrap delivery was
Shared `f295a4a6b`, NXVM `5fc82badb`, MyRaidenII `585cbabd7`.
[Bootstrap evidence](../etc/evidence/m0-s1-bootstrap.md) preserves its original
scope/proofs. T1 supersedes M0's unconditional unchanged-Lib reuse assumption;
it does not rewrite the historical acceptance or authorize Shared changes.

## Queue

[Queue](QUEUE.md) contains six unadmitted implementation candidates beginning
with V30. Only T1 is currently admitted, under the owner's standing MyRaidenII
implementation objective and single-session dual-role execution.
