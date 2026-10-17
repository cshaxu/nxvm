# T2 S3 Native Family Delivery

Baseline `0e1a32fe4`. This delivery covers the native-family batch, not complete
V30 qualification. The active packet retains the complete brief until review.

Implemented at the sole X86 Core CPU owner:

- Append V30 as value 7, preserving all previous profile values and layouts.
- Replace the remaining ordinal capability/admission assumptions with private
  explicit family predicates; Machine remains explicitly Intel-only.
- Native reset FFFF:0000, cleared SS/DS/ES, MD set and direction/interrupt/step
  cleared; six-byte V30 prefetch capacity. Other undefined reset values are
  deterministic initialization, not claims of hardware-defined values.
- Native PSW load/image and debug/entry preserve MD. Fixed PSW image bits 14--12
  are one. Native debug cannot manufacture Intel FS/GS/control registers.
- Compatible 16-bit forms use existing handlers, execution and preview, never
  a copied V30 interpreter or Intel protected/32-bit dispatch.
- Pending NEC/8080 execution and physical timing are not qualified. Unsupported
  NEC forms stop diagnostically, not via a fabricated native INT 6; emulation
  bytes cannot silently execute as native. Timing is unattributed/unallocated
  with no invented Intel clock cost, and Machine refuses V30 creation.

Source: T1-pinned original NEC October 1986 manual. PDF pages 16 and 30 were
rendered and inspected: printed sections 3-3 PSW image and 7-1 reset. Section
3-4 describes the six-byte V30 queue; the MD write-enable/mode-return machinery
of section 8 and section 12 remains a later complete mode batch. No external
implementation, decryption table or ROM data was imported.

The new owned CPU test uses an actual reset fetch/far jump and a synthetic
program exercising register/memory/stack operations, arithmetic, loops,
port completion and HLT. Its semantic terminal/register/memory assertions,
not its containment step budget, prove the result. Separate groups test
enum/capabilities, reset/entry, native PSW, preview/truncation, exclusion of
Intel 0F/32-bit forms, unimplemented emulation and unqualified timing.
The Machine profile test proves V30 still cannot be published as a machine.

Current actual code delta, excluding manifests/docs/artifacts: production
+341/-239/net +102; tests +220/-0/net +220, including the new 206-line CPU unit.
The estimate was production +180--300/-230 and tests +200--350. Most changed
production lines are predicate substitutions; five private inline helpers
own actual family/form/prefetch decisions. No new public function, callback,
structure, state mirror, worker, exception model or alternate runner is added.

## Verification And Boundaries

- X86 Core source/test manifests, dependency/Types/ownership gates and existing
  verifier negative with the new ordinal-capability case pass.
- Entire X86 Core unit group: x64 167/167 (57.60 seconds), x86 167/167
  (17.37 seconds), including all old Intel profiles and the new native test.
- Initial full-repository builds stopped at the existing X86 NXVM Board reset-alias
  switch: its formerly exhaustive Intel enum has no default, and adding V30
  triggers `-Werror=switch` on both compilers. No warning was suppressed.
- The owner approved moving that helper's existing trailing `return 0u`
  into `default: return 0u`, plus its manifest. This preserves every old result
  and adds no PC/V30 behavior. The source sweep found no other component-external
  production switch directly over CPU profile. The narrow edit and its manifest
  are now applied; both complete repository builds pass without suppression.
- Complete repo-only units pass on both original Release Ninja/ccache graphs:
  x64 656/656 in 230.89 seconds, x86 656/656 in 150.10 seconds. No retry,
  test-source/expectation change or increased budget was needed. Groups include
  all six Shared packages, five existing Apps and static/negative checks.
- The existing eight PC 0563 artifacts are rebuilt in their receiving graphs,
  restoring default selection after the four product links per architecture.
  No CPU rebuild is required between product links. The artifact identity/hash
  [record](t2-s3-pc-artifacts.json) accompanies this delivery; PE architecture,
  product name, version and adjacent INI identity all pass. MyNES has no CPU dependency and its pair,
  all user INIs and external firmware/media remain unchanged.
- No new V30 Machine, desktop/RDP, real-ROM or gameplay qualification occurred.
  Background CTest/build results do not assert those unexecuted validations.

The retained path is the existing metadata/handler executor with private family
decisions; no copied interpreter or new state owner exists. The five inline
helpers own actual repeated capability and prefetch decisions, not forwarding
facades. One public enum value is appended; no new function, type or layout is
introduced. Initial full-build enum-consumer failure is retained above, not
erased by the approved equivalent fallback repair.

The similar-issue sweep uses the ledger's ordinal query over CPU/Machine C/H
(zero remaining hits), all production profile switches (only the approved Board
consumer outside X86 Core), and reset/name/entry/debug/PSW/time seams. Existing
Intel predicates preserve their truth sets for profiles 1--6. The local corpus
gate and its negative test reject ordinal capability regressions. Complete
instruction/mode/timing proof remains in the original V01--V17 ledger, rather
than being inferred from the representative native program.

Shared source/test/manifest delivery is `da7e39b7d`; NXVM artifact delivery is
`3af1cfcf1`. Both were pushed immediately. The
compiled instruction owner SHA-256 is
`C408B16AB2D8A56AA88AE43335FB596D2037DA3591FF8E67215307B4C13212F8`.
The artifact record identifies the eight receiving files built from that source
at the unchanged PC version 0563; MyRaidenII's T2 is not a PC release version.

The original V30/game objective, all incomplete ledger rows and owner-declined
Lib colour gate remain intact. Per-target source/artifact/evidence commits are
followed by separate coordinator actual-change review before S acceptance.
