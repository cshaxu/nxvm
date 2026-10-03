# T540 S91 Complete PIC Aggregation Evidence

Baseline: b44ac5dee, clean accepted S90. S91 is accepted after actual-pushed-diff review.
Local project-owned PIC source is authorized under the existing root MIT
policy; no external source or asset is imported. Existing copyright notices
remain with the moved implementation.

The intake query is `rg -l` over tracked source/test C/header files for
`shared_pic_master|shared_pic_slave|core_machine_pic_bus|core_machine_pic_irq_source|pic_bus.h|pic_bus_interface.h`.
It finds fifteen production and sixty-five test files. Every live member
belongs to the current whole PIC receiver; no per-field preparation is a
delivery. DMA, family wiring and genuine D4 remain under T540's ledger.

At baseline, App embeds endpoint and asserted-source state in the board and
controller attachments. The Shared receiver instead owns opaque endpoints
and source leases for one lifetime, preserving the original aggregation,
cascade, port, reset and timing algorithms. Initial lease failure propagates
through the owning construction operation; reset/reconnect reuses the lease.
No private chip getter, App facade, second registry or hardware model is added.
The sections below record complete proof, final artifacts and acceptance.

## Implementation Progress

The PIC implementation/private layout/public interface are physically moved to
`x86/ibmpc-common`. App controller attachments now borrow opaque endpoints and
source leases. The pair owns and destroys leases; initial allocation failure
propagates through PIT, KBC, XT keyboard, RTC, FDC and HDC construction. The
production Core library compiles with the existing strict warnings.

Four original pure PIC tests and the PIT-to-PIC test move with their owner;
their fixtures construct Core through its neutral public contract rather than
embedding a peer's private Core layout. App CPU fixtures share the relocated
PIC test helper. A lease is invalid after pair destruction: old fixture reuse
has been corrected to clear borrowed handles between fixtures.

The OCW3 migration exposed a diagnostic side effect: reprogramming OCW3 through
the port-equivalent refresh path could republish cascade inputs while merely
inspecting IRR/ISR. A minimal copied three-register observation now belongs to
the chip and is exposed through the aggregation owner. It changes no chip
algorithm, timing, polling or acknowledgement semantics and exposes no handle
or mutable layout. The shared helper uses that observation; real poll reads
still use Core bus dispatch. All four migrated original PIC tests pass. The
original NXVM PIC transaction-phase, KBC controller and FDC regressions also
pass; the boot matrix probe compiles. This is intermediate proof only.

The three remaining CMOS/FDC tests no longer read source-private endpoint
fields; board endpoints and the borrowed source handle preserve their original
observations. A same-owner lifecycle seam proves endpoint allocation failure
publishes no handles, source allocation failure publishes no lease, retry
succeeds, and rebind reuses the existing lease even when allocation is disabled.
Its standalone regression passes. This seam changes no production API. The
complete independent x86 suite passes 135/135, including 131 unit cases.

The first full x64 unit run passes 471/472. Its only failure is the CPU-bus
negative verifier retaining the old address-of endpoint spelling. The verifier
now requires borrowed opaque endpoints and rejects App imports of the private
PIC aggregation header, raw chip access and source layout access. Its negative
regression passes. A complete latest-source rerun is still required; this first
run is not acceptance proof.

Both complete root unit suites now pass 472/472. The independent suite passes
135/135 and all six source/test manifests verify. Documentation governance
passes. The relocated five tests retain their canonical names in the product
acceptance inventory while Shared alone registers them; the dual-registration
gate passes after correcting that inventory omission.

Direct baseline-to-worktree body comparison, ignoring whitespace, confirms
eleven original operations are unchanged: refresh, reset, set IRQ timing,
advance, deadline, assert, deassert, timer output, scan, peek and acknowledge.
The new work changes endpoint/source lifetime and publication, not those
algorithms. App has no remaining x86_pic_* call, so its redundant direct PIC
chip link is removed; the aggregate owns that dependency.

## Final Product Verification

All eight final Release products are rebuilt from the PIC extraction. PE
architecture, 0.5.0540 banner, absence of compiler debug sections and input
freshness checks pass. Every actual generated link response file contains
exactly one production x86-core archive and no observable Core archive.

Each unchanged-INI boot checkpoint was run once, sequentially, from its final
profile build: default x64/x86 reaches the DOS prompt; XT, AT and Model 40
x64/x86 reach the running installer. All eight neutral-link probes pass.
No owner INI content, media, MyNES source, tests or executable changed.

The final test-only vocabulary cleanup replaces stdio/printf in the six
Shared common tests with the existing Lib Types file interface. Output and
assertions are unchanged. Independent tests pass 135/135 on this final tree;
complete root unit/gate reruns pass. The deployed products require
no additional rebuild for this test-only cleanup.

Implementation delivery is Shared P1 `98fd9460b` and NXVM P2 `f19d84304`, both
immediately pushed to origin/master. Governance P3 closes only S91; T540 stays open.

Actual fixture review finds one missing explicit PIC teardown in the Compaq
HDC direct fixture. Both normal and failure exits now destroy the borrowed
pair after its producers. A repository sweep of direct PIC initializers finds
no other file lacking a matching finalize operation. This test-only correction
must pass the final complete units before delivery; it does not change products.

The independent lifecycle regression also exercises a last-port collision in
both single and cascaded topologies. Initialization returns INVALID_STATE,
publishes neither endpoint, retains the pre-existing port and leaves no earlier
PIC route installed. This directly verifies the PIC-to-Core atomic publication
contract rather than relying only on Core's general registration tests. The
regression passes; final complete suites are rerun after this addition.

Coordinator review inspects the actual pushed source/header, build, fixture,
test, artifact and document differences against b44ac5dee. It confirms the
single aggregation owner and variant-selecting composition; publication and
construction failure handling; producer-before-pair teardown; non-mutating
diagnostics; preserved hardware assertions; and no App copy, private chip
getter, mirrored source state or unrelated consumer change. Commit target
lists are disjoint Shared/NXVM and the reviewed tree equals origin/master.
The complete PIC ledger batch is accepted. Documentation review checks the
packet, numeric S/P order, retained task status and truthful residual owners,
not merely the structural documentation gate. The next whole receiver is DMA.

## Final Delivery Proof

The final tree passes independent tools-enabled tests 135/135 and the separate
tools-disabled tests 129/129. Complete repository-only units pass 472/472 on
x64 (60.55 seconds) and x86 (57.23 seconds), after the final fixture cleanup and
route-collision regression. Both 81-check specialized runs pass, including
the strict compilation matrix (402 rows, 381 strict and 21 existing deferred)
and canonical test-registration checks. Six manifests verify using each
explicit LIBRARY_ROOT; documentation governance and git diff --check pass.

Tracked C/header numstat against b44ac5dee, restricted to src/ and test/ and
excluding documents, scripts and artifacts, counts 106 rows: +2,076/-1,852,
net +224. The count includes moved files and all actual fixture consumers.
The increase pays for opaque lifetime/allocation failure handling, copied
diagnostics and independent failure/rollback regressions, not a framework or
another chip/board implementation. The eleven original algorithm bodies
remain unchanged. Existing direct fixtures and canonical hardware assertions
are retained rather than removed to make the relocation pass.

The requirement-to-proof mapping is:

- Physical single owner: Shared target compiles pic_bus.c; App source and
  direct PIC-chip linkage are removed; the App archive contains no PIC adapter.
- Public boundary: opaque endpoints/source leases, copied topology/timing and
  register observation; CPU/PIC authority negatives reject private imports,
  raw chip calls and source layout access.
- Lifetime/failure: all producer construction statuses propagate; reset/rebind
  reuses leases; pair teardown owns all allocations. Independent allocator and
  both-topology last-port collision regressions directly prove rollback.
- Behavior/coverage: eleven original bodies compare equal ignoring whitespace;
  original PIC and board assertions remain; independent and both full suites
  pass. Timing classifications and hardware algorithms are not upgraded.
- Production delivery: eight stripped 0540 products have one Core link each;
  all eight unchanged-INI semantic checkpoints pass once. The table below
  identifies the exact products. MyNES and owner configuration are unchanged.
- Convergence: the complete PIC ledger batch is delivered; DMA, board assembly,
  AT/XT wiring and D4 remain explicit T540 receivers, not S91 completion claims.

## Review And Coverage

The changed production class is the PIC pair/source owner plus every actual
App producer, board advance/deadline/reset/destruction caller and declaration.
Chip algorithms are unchanged; the only chip addition is copied diagnostics.
The retained port adapter and source counters have one Shared implementation.
No forwarding App file, duplicate chip compilation or private handle getter
remains. Construction failures are returned before device publication and the
board candidate is destroyed; reconnect/reset keeps the borrowed lease.

Caller/test review checks pointer conversion, source initialization between
distinct pair lifetimes, callback contexts, initial allocation status, copied
assertion observation and teardown. CPU/exception assertions and original PIC
hardware expectations remain; direct chip mutation in App fixtures is replaced
by actual register/source operations. The independent lifecycle test alone
uses its own private layout and allocator seam. A duplicated null-source
assertion introduced by mechanical migration is removed before delivery.

The intake query's fifteen production/sixty-five test hits were not the full
final edited surface: field-only and fixture callers extend the same class.
The final source/test count includes all those callers and relocated tests,
not a new scope or a weakened coverage universe. DMA, common board assembly,
family wiring and D4 keep their existing T540 receivers.

The similar-issue sweep uses `rg` over `src/app-nxvm` and its tests for old
PIC includes, mutable endpoints/source fields, chip getters and raw chip calls;
the CPU/PIC and controller authority gates reject their production recurrence.
Every direct PIC initializer's file is also checked for owned teardown; the
Compaq fixture correction is the sole missing-finalize hit. Core route rollback
and source allocation/reuse now have direct independent regression evidence.

### Final Products

Paths are relative to `assets/nxvm/`; all products retain revision 0.5.0540.

| Profile/product | Bytes | SHA-256 |
| --- | --- | --- |
| compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe | 1336504 | 805F8E00234CF343129BE614048A4C254F618B06611D8FB3A204EF91F397EF9E |
| compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe | 1506832 | 8EDB1063B37541EA94A45023D79AF39859A7AAA287135EE963DBCD27D0EBB50E |
| default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe | 1352821 | 42FF3767A0FB59A556C7DE5C0F5A6B55ED98F3181F2BC4A94107CBDBD921CA3A |
| default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe | 1523147 | 9560909907445ED7B18B748705E874596C36DC548A130DECD48FFB0CFA71C7A5 |
| ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe | 1352789 | 8C700F490E94F9639379CBBBB2EE9B99C572FF40DE89880B73059D3140D04B8B |
| ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe | 1523114 | B3F4C3028C97674A3F31208736AF9F295DAA9C04EB0A150EBFFEE8C4AA2C54BC |
| ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe | 1352855 | DABDD1404C0140740A47A56B93AE4EED48C51DB10FDC74028154FE12D8C6E476 |
| ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe | 1523182 | 4D9848337E621AE0EA944556A0D349065F1BCC190B66387ABA55667FE1736506 |
