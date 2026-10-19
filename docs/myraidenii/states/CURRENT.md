# MyRaidenII Status

## Active S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S3. One session performs sequential coordinator and executor roles. |
| Admission And Approval | Owner accepted S2 and approved entry to the next S on 2026-10-19. S2 P1--P3 (`dd2611e1b`, `9f941ae69`, `f2c30a1e6`) are the accepted baseline; the frozen B01--B09 ledger admits this bounded B04--B06 batch. |
| Objective | Implement MyRaidenII-local board main-IRQ acknowledgement, input-register state and main-to-sound command/status latches through the existing X86 Core Machine attachment and board memory routes for B04--B06. |
| Non-goals | No Shared public API change, external ROM bytes, MAME code/table import, periodic VBlank source, Z80/YM/OKI execution or host audio, COP/DMA behavior, video/Product implementation, game bypass, executable or gameplay claim. |
| Reference Baseline | Clean accepted S2 P3 `f2c30a1e6`; accepted T3 S1 ledger/evidence; archived T2 V30 and neutral Machine public qualification; current X86 Core attachment and memory-device interfaces. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../history/M1-T3-board-cop.md; ../etc/evidence/t3-s1-board-contract.md; ../etc/t1-freeze-ledger.md; ../etc/research/t1-integration-contract.md. |
| Files And ABI Surface | MyRaidenII-local Core board source/header and its existing focused ROM-free board test family. The local board interface may gain only the minimal typed operations necessary for a later Machine/Product owner to set input state or issue a board interrupt; no Lib/Emulator/X86-Core public interface or asset ABI. |
| Applicable Rules | Execution packet/test-closure and one-target P rules; Architecture Product->Machine->Core, sole X86 Core timeline and public bus routes; Coding test-owner/Types rules; source policy prohibits game/third-party import; build hygiene. |
| Verification | Inspect existing attachment and MyRaidenII board-test ownership before adding coverage. Run focused B04--B06 vectors through the public Machine on x64/x86 plus the complete repository-only unit suite before S closure. Run corpus/manifest/dependency gates for touched owners. No external-ROM startup, desktop or gameplay test is claimed. |
| Expected Markers | One board attachment owns pending/acknowledge state; one board input-register representation has explicit active polarity and reset state; one eight-bit command/status latch owns main-to-sound handoff; all delivery uses existing Machine attachment or memory routes. |
| Asset Needs | ROM-free byte fixtures only. External owner-ROM inventory metadata is read-only; no ROM copy, extraction or repository asset change. |
| Reporting Requirements | Record every B04--B06 address/polarity/vector fact and source grade, every added/extended test and why it cannot reuse an existing owner test, line delta, public-interface decision, x64/x86 verification and deferred B07--B08 receivers. |
| Stop Conditions | Stop before any Shared contract expansion, unresolved source fact that cannot be honestly expressed as an L2 board model, required external-ROM data copy, or a need for a periodic raster/sound clock, second bus or second timeline. |
| Exit Criteria | B04--B06 code and focused ROM-free vectors are complete; no duplicate CPU/generic-Core test path is added; required x64/x86 focused and repository-only unit checks pass; documentation/evidence/manifest checks pass; one-target commits are pushed. |
| Original Owner Request | Implement MyRaidenII under single-session dual-role governance; reuse existing coverage where it owns the same responsibility and do not inflate tests through redundant copies. |
| Similar-Issue Sweep | Inspect all MyRaidenII-local board attachment and memory routes for duplicate interrupt/input/latch ownership; compare added tests against existing CPU interrupt, Machine attachment and generic RAM owners, retaining only genuine board-specific coverage. |

M1 T1 and T2 are closed in [history](../history/). T3 S1 and S2 are accepted;
S3 is active for B04--B06 board routes only. No runnable MyRaidenII executable
or gameplay result is claimed.
