# T539 S6: DMA First-Service Phase Repair

Baseline 4f278a918. This NXVM-only prerequisite consumes the first-service
ordering portion of the DMA ledger row. It does not mark dma.c/dma.h extracted,
qualify the entire controller, or modify Shared, MyNES, firmware, media or INIs.

## Direct Finding And Authority

`core_machine_dma_advance_one()` selected secondary channels 1-3, called
`dma_service_begin()`, then immediately called `Execute()`. Primary channels
instead returned after beginning service and progressed through
`dma_service_advance()` on subsequent DMA clocks. Later secondary transfers also
used that phase function. The first secondary transfer thus bypassed its own
normal/compressed state machine and exposed memory/provider/TC effects early.

Intel 231466-005, *8237A High Performance Programmable DMA Controller*,
September 1993, printed/PDF pp. 4 and 7 were extracted and visually reviewed
from the existing external `manuals-nxvm/dma/intel-8237a-dma-controller-sep1993.pdf`.
SHA-256: `2A1AC5BBF5B4BE75BA9272647EFFB389BCC5E76CBFE293D5C56E75217E2BC8A8`.
Page 4 defines normal working states and the two memory-to-memory halves;
page 7 distinguishes S4 transfer from S1 address-latch update and S2 address
change, and removes S3 for compressed timing. There is no first-transfer
exception for the PC/AT word controller. No external source was imported.

The blanket D7 success conclusion in the [T507 audit](t507-s4-dma-controller-reaudit.md)
missed this caller. This repair restores the intended phase ordering, rather
than downgrading the incorrect path into an accepted compatibility model.
It makes no new claim about physical clock calibration, pin-level accuracy or
other unexamined DMA behavior.

## Repair And Complete Affected Family

Delete the direct `Execute()` call from secondary arbitration. The existing
phase handler becomes its sole caller, with three effect boundaries: normal S4,
memory read S14 and memory write S24. No new state, public API, delay constant,
mode branch or scheduler is introduced. Address/page conversion, transactions,
callbacks, priority, masking, cascade selection and failure order are unchanged.

The existing channel smoke gains a table-derived 126-case cross product:

- All bindable global channels 0-3 and 5-7; channel 4 remains board cascade.
- Normal and compressed timing.
- Demand, single and block modes.
- Existing verify, device-to-memory and memory-to-device behaviors.

The test drives the production clock-phase entry, not the accelerated fixture
helper. It checks each pre-transfer tick for unchanged provider counts, RAM,
current address/count and TC; then checks one transfer and one terminal callback,
direction/width-correct data, address/count advancement and no repeat completion.
It does not inspect private phase fields or add a test-only public observation.
Verify's existing provider handshake is retained, not newly hardware-qualified.

Negative control: adding the matrix to the unmodified baseline produced exactly
54 failing rows (all secondary cases); all 72 primary cases passed. With the
one-line production deletion all 126 and the original channel scenarios pass.

## Similar-Issue Review And Remaining Extraction Work

Inspected every `Execute()` caller and both arbitration branches, M2M halves,
`core_machine_dma_advance()` and `core_machine_dma_advance_transaction()`, plus
scheduler HOLD/READY/clock publication. After repair all real effects run through
the existing phase handler. The accelerated entry is test-only, not another
machine execution route; its removal from production belongs to DMA extraction.

Extraction still must separate page/lane expansion, physical RAM/transaction
access, binding identity/provider lifetime and pair wiring from the opaque chip.
In particular, current master-clear also clears the colocated board page state;
that behavior must receive an explicit disposition rather than disappear during
a struct move. This S neither changes nor accepts that separate reset contract.
The finite ledger remains pending for DMA migration; S6 only removes the phase
bypass that would otherwise complicate its neutral grant boundary.

## Verification And Delivery

- Full NXVM units: x64 341/341 (197.97s), x86 341/341 (30.94s).
- Default-profile integration: 20/20 per width (12.22s/12.39s).
- Each remaining profile/width boot passed once: XT 18.87s/22.87s,
  5170 33.18s/43.37s, Model 40 64.81s/76.96s (x64/x86).
- Specialized static aggregate, six shared manifests, documentation governance,
  58 changed-document local links and diff whitespace checks pass.
- All eight optimized Release EXEs have 0.5.0539, correct 8664/014C PE machine
  and no .debug/.zdebug/.stab sections. Runtime Debug remains intact.
- Four INIs, all six shared corpora and MyNES source/tests/docs/artifacts have
  no content diff. Temporary PDF page images are removed; the original remains
  external and untouched. Reusable dual-width trees are restored to default
  configuration for the next S.

Commands: `cmake --build build/t539-s3/nxvm-<width> --target run-unit-tests`
and `run-integration-tests`; reconfigure NXVM_PRODUCT_PROFILE and build
`vm-0-5-0539` plus `vm-profile-floppy-boot-matrix`, then run that CTest row once
per remaining profile/width. PE headers, embedded version and objdump section
tables are checked on the deployed assets/nxvm files, not a stale build copy.

Counted against baseline with `git diff --numstat` over production/test C/H:
one production file +0/-1 (net -1); one test file +135/-1 (net +134).
Docs, build output and binaries are excluded. The test growth provides one
shared fixture/case generator for 126 combinations, not 126 copied fixtures.
No original test scenario or expectation was deleted or weakened.

| File | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0539_x64.exe | A651C5DEFFAC4758EF3101F3CFB3C5D3FAA8BD779726E596B18CA9490D76B21D |
| nxvm_model40_0_5_0539_x86.exe | 0986804572DF73AC7931082E31BEFB96C926B9ABAAACD820710E28F6DED82916 |
| nxvm_default_0_5_0539_x64.exe | 28B56AE2E71A5073BA26E65BB425DACA4A1DF2D80FA7E30618E9F21CD21BD0D4 |
| nxvm_default_0_5_0539_x86.exe | C26E961629E906F59145A0D39491BB4F2FC05573B11C7B686C27173BC10F6702 |
| nxvm_xt_0_5_0539_x64.exe | 2C14C29DF79D68DB20F625BC9B0499F2B09F22FEF416D83B7513E630011F6DFD |
| nxvm_xt_0_5_0539_x86.exe | 4166B65E856011001CF70BF88D1CC353DFB33B76C0DD389139AB9F41773DC66B |
| nxvm_at_0_5_0539_x64.exe | AB56A4D9CD23948699C246AA18852C786FD7B6B4190A7A737C0204FE0BD256E5 |
| nxvm_at_0_5_0539_x86.exe | BA93DE5AEC36A99FDAFB047D5AE6CF999E0E9E0C09DBCF28EC75161979883A6B |

## Actual-Change Acceptance

Coordinator-role review of pushed 0746220bf inspected the production deletion,
all matrix code, packet, ledger, corrected historical claim, build identities and
remaining-document diff against the bounded brief. The existing phase handler
is the sole transfer owner; no register/phase implementation, API or product
branch was added. The matrix uses visible effects rather than a duplicate phase
model, preserves original tests, and proves the baseline failure before success.

Every first-service family member has direct proof; the unextracted DMA row and
its separate page/reset and board-boundary work remain explicitly pending.
One NXVM-target P contains the complete implementation and required evidence;
Shared and MyNES are unchanged. S6 is accepted and its active packet is removed.
T539 remains open for automatic admission of the next bounded extraction batch.
