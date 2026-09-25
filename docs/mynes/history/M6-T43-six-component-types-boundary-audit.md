# M6 T43 Six-Component Types Boundary Audit

## S1 Admission

Owner admitted S1 on 2026-09-24. It freezes and classifies the entire six-root
Types boundary without modifying Shared code. Its evidence ledger is the
durable T43 convergence record; later repair work requires owner review before
any Shared source or ABI change.

## S2 And S3

S2 delivered `51d6fea52` and `938899d2b`; owner review identified residual
pointer vocabulary and an incomplete gate. The owner admitted corrective S3.

S3 implementation `f5a9bd34a` fixes the entire observed batch and the wider
scalar/constant/function sweep. It also fixes relative-root empty scans and
manifest hashes that depended on CRLF working files. Coordinator actual-diff
review accepted the unchanged runtime/test semantics, bounded declaration
additions, negative gate coverage and committed-export verification. Both
complete configured x64/x86 suites passed 130/130; the 0043 artifact pair was
rebuilt. S3 is closed; T43 remains open for owner review.

Detailed proof: [T43 ledger](../etc/evidence/m6-t43-s1-six-component-types-boundary.md).

## S4 SoftPC Import Audit

Owner admitted the six-root comparison on 2026-09-25. Delivery `87a45b7dd`
records all 227 paths and all 17 differences against clean SoftPC `dc9c34ce`.
Actual-change coordinator review accepts the unchanged-source import verdict:
the candidate adds bounded 80x50 text capacity, preserves Types and existing
canonical capabilities, and requires a unified receiving rebuild for its changed
text layouts. Existing MyNES/NXVM 25-row producers remain source-compatible;
NXVM 50-row hardware support is not supplied by this change.

Six manifests and boundary checks pass; isolated Common x64 tests pass 18/18,
changed Lib tests pass 6/6, NXVM frame probes pass both widths and MyNES driver
compiles against candidate headers on both widths. Documentation governance and
whitespace checks pass. This audit-only S changes no Shared source or binaries;
product runtime acceptance belongs to an admitted import. S4 is closed and T43
remains open. [Complete audit](../etc/evidence/m6-t43-s4-softpc-import-audit.md).

## S4 Follow-up And S5 Acceptance

The owner subsequently authorized same-S import and delivery despite the
reported physical audio test failure. Shared `83022ea9f` and MyNES `f67eaefed`
preserve that import and its explicit non-green result; S5 owns its correction.

S5 Shared P1 `b7cbb30a9` replaces physical loopback with deterministic execution
of the real WASAPI adapter. MyNES P2 `3959be3de` records full evidence. Coordinator
review inspected the actual test replacement, CMake, manifest and documents;
no production or public API changes, host changes, optional mode or sibling
writes occurred. Complete suites pass on x64/x86: Shared/MyNES 130/130 and
NXVM units 335/335. Six manifests, Types and documentation gates pass. The
0043 pair was incrementally built and retains its verified S4 hashes.

The original failure's full batch is resolved at the test acceptance boundary,
not hidden as a skip or claimed as physical audio certification. Source/test
delta is +217/-209, net +8, excluding manifest/documents. Architecture/coding
governance guided the OS-boundary test rather than a production workaround.
S5 is accepted and closed; T43 remains open for owner review, without an active
packet. [S5 evidence](../etc/evidence/m6-t43-s5-native-audio.md).

## S6 Receiving Artifact Delivery

Owner requires delivery governance and missing receiving EXEs, clarifying that
unaffected executables need no artificial rebuild. Shared P1 `e25aec65b`
requires impact review of every receiving App and current product builds,
not merely test builds. NXVM P2 `735d155a9` delivers eight refreshed 0535
executables (four profiles, x64/x86) and [hash/build evidence](../../nxvm/etc/evidence/m6-t43-s6-receiving-artifacts.md).
MyNES 0043 targets build incrementally with unchanged S5 hashes; existing
version identities are preserved. Production/test source delta is zero.

Full S6 verification: NXVM unit 335/335 each width (23.69/22.77 seconds);
Shared/MyNES x64 130/130 (130.51 seconds). First x86 run passed 129/130,
failing `library.kvm_window_modal` at its pre-freeze modal-exit assertion.
Three unchanged isolated reruns passed; complete unchanged x86 confirmation
passed 130/130 (153.20 seconds). The initial failure is not erased: a P2 TODO
requires owner-reviewed Shared investigation before altering this native test.
Cause is not proven. Both product documentation gates and whitespace pass.
No new external-ROM integration or user-visible manual acceptance is claimed.

All eight rebuilt NXVM EXEs have verified PE architecture, version and no debug
sections. Source, shared tests, MyNES binaries and user configuration were not
intentionally changed. After product builds finished, the default NXVM INI
changed externally from `console_control=false` to `0`; this unrelated edit is
preserved and excluded from commits, so a completely clean worktree is not
claimed. Owned temporary logs are removed after results are recorded; reusable
build trees remain for open T43. S5's MyNES banner typo is corrected to 0.0.0043.

Coordinator actual-change review accepts P1/P2 and MyNES P3 `0c7a58792`:
rules distinguish necessary artifact refresh from test-only churn, all receiving
profiles are accounted for, hashes/PE results and final complete suites are
recorded, target commits are separate, and the native-test risk has a named
receiver. S6 is closed; T43 remains open with no active implementation packet.

## S7 Shared Quality Repair Acceptance

Owner approved the bounded assembler, Audio, Common synchronous-request and
corpus consistency repairs, including existing cancellation/join on completion
failure. Shared P1 `38ed0f26f`, NXVM P2 `6db4a9298` and MyNES P3 `6682a341c`
are pushed. Coordinator actual-diff review confirms one target per commit,
unchanged public API, no product-source or INI edits, original assembler tables,
one FIFO/worker owner, and error paths/tests matching the approved design.

Final frozen-source verification: Shared/MyNES 130/130 on x64 and x86
(156.83/158.36 seconds); NXVM units 335/335 (61.40/61.15 seconds). Six manifests,
Types boundaries, documentation and whitespace pass. All ten affected EXEs are
rebuilt, architecture/stripping checked, hashed and committed. Source/test count
is +350/-99, net +251; the increase is failure handling and regression coverage.
[Evidence](../etc/evidence/m6-t43-s7-shared-quality-repairs.md) maps every batch
member and the retained native synchronization failure boundary.

Similar asynchronous wake/control-completion failure contracts are recorded in
TODO for owner-reviewed Shared admission, not silently described as solved.
No new external-ROM NXVM integration or owner manual acceptance is claimed.
Prevention: retain injected failure/partial-delivery regressions and require
receiving product-target builds rather than test-only rebuilds. Reusable build
trees remain for open T43; only the unrelated pre-existing default INI edit is
left uncommitted. S7 is accepted and closed; T43 stays open awaiting the owner.

## S8 Quality Repair Delivery

Owner approved the complete audited A-F batch, including public boolean layout
changes, small owner-local simplification and test registration reuse, plus
review/submission of the owner's MyNES and NXVM.ini edits. Shared P1 bfcbd31b2
and NXVM P2 14403bf51 are pushed. Actual-diff review confirms unchanged assembler
tables/command flow, retained native numeric contracts and sole queue/resource
owners. Shared code delta is +706/-776 (net -70), including the new helper.

All configured Shared/MyNES tests pass 130/130 on both widths; NXVM units pass
335/335 on both. Independent Shared suites pass 49/49, 18/18 and 10/10. Six
manifests, Types, DAG, corpus, documentation and whitespace checks pass. Ten
optimized stripped artifacts are rebuilt and hashed. The owner's MyNES title
and formatting edits are preserved and tested. See the
[repair and receiving ledger](../etc/evidence/m6-t43-s8-six-corpus-quality-audit.md).

The owner's default INI uses console_control=0, but NXVM currently accepts only
true/false. This is a real failed validation, not a Shared regression. The owner
was asked whether to accept numeric aliases in the existing parser or restore
false. Until that decision, the INI is untouched/uncommitted and S8 remains open;
no clean-worktree, owner hand-test, S acceptance or T closure is claimed.
