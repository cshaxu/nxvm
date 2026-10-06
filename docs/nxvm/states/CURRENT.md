# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T544 | Closed as CPU audit; complete repair/proof findings transferred to the first queued proposal, not claimed repaired. |
| M5 Td S177 | Complete: CPU audit closure and full repair transfer, archive/queue/reference reconciliation. |
| T545 | Closed after S7 actual-change acceptance: fixed eight-corpus import, preserved receivers, four owner-local test packages and full receiving qualification. No active S packet. |

## Accepted T545 Baseline

S7 implementation P1 is `90e91d721`, pushed to origin/master. Coordinator
actual-change review accepts the entire original request, S1-S6 ledgers and
original S7 verification, not merely a passing unit summary. See
[final proof](../etc/evidence/t545-s7-final-qualification.md),
[task history](../history/M5-T545-softpc-eight-corpus-refresh.md) and
[archived proposal](../history/M5-T545-softpc-eight-corpus-refresh-proposal.md).

All 58 original PC integration contexts pass once with unchanged checkpoints:
default 22/22, AT 3/3, Model40 3/3 and XT 1/1 per width. Complete PC/shared
units pass 532/532 per width; MyNES receiver units pass 43/43 and integration
12/12 per width. Eight manifests, 33 supplemental checks per width, both
specialized aggregates, artifact-root/INI and documentation checks pass.
S7 corrects only two NXVM static fixture checkers after S5 relocation; their
44-owner/133-constructor inventories remain intact. No Shared production,
API, App source, owner INI, media or executable input is changed by S7.

S2 imports exact committed SoftPC
`8124e551e841ccdec2ceb7f6a0f6ae5b513a7951` eight-root bytes. S3-S6 subsequently
complete Lib/Common/x86/IBMPC owner-local tests with refreshed test manifests;
current test bytes are not claimed identical to that historical upstream pin.
All preserved/relocated assertions and removed-file dispositions remain in
the S1/S2 inventory and S3-S6 evidence linked by task history.

## Current Technical And Runnable Baseline

- Independent chips and sole CPU implementation live in x86/chips; neutral
  execution, memory/ports and guest time live in x86/core.
- IBMPC board-common/AT/XT own PC wiring; Machine owns one Common driver and
  pacing/media adaptation; Product owns one INI/command/Debug/UX/entry path.
- Four fixed Apps own immutable compositions and firmware bindings. Model40
  alone owns D4. No App production graph links a peer App.
- Shared tests follow their owners; actual model assertions stay App-local.
  The one external family harness executes each real fixed binding and its INI.
- Eight optimized stripped PC 0.5.0545 EXEs remain directly in assets/my5160,
  assets/my5170, assets/mydeskpro386 and assets/nxvm, with unchanged adjacent
  owner INIs. MyNES retains its two 0.0.0043 EXEs in assets/mynes. All ten
  hashes match S2 evidence; PE widths and absence of compiler debug sections
  are verified. Runtime Debug remains. No test-only rebuild is manufactured.
- External media masters remain unchanged after overlay integration. No owned
  build/test process remains. Ignored receiving caches remain needed for the
  immediate CPU repair's incremental regression checks.

## Next Work And Qualification Boundary

The owner requests CPU instruction/function/timing repair next. The first
[Queue candidate](QUEUE.md) is [CPU gap repair](../proposals/m5-cpu-audit-gap-repair.md),
with eighteen coherent mechanism/proof batches and final qualification.
Its next numeric T and S1 packet are not allocated by T545 closure.
Concrete Shared repair review remains required by that proposal.

T544's [complete audit](../etc/evidence/t544-s7-five-family-convergence.md)
and family findings remain intact. T545 imports/tests/boot qualification do
not resolve them or establish complete CPU correctness, new L3 timing or a
physical-time axis. Existing Common wake/Console rollback, portability and
other debt remain explicitly in [TODO](TODO.md), not claimed fixed here.
