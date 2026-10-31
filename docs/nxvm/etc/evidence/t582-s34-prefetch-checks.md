# T582 S34 — instruction prefetch check consolidation

`ExecInit` previously invalidated prefetch once for an expected-PC mismatch,
then invalidated it again when the now-invalid buffer entered the refill
branch. The candidate combines those admission conditions and performs the
same refill after one invalidation. The invalidation's possible memory-lock
completion clears the active bit before its callback; the second call has no
additional lock to complete under the existing no-CPU-reentry contract.

The subsequent copy no longer repeats the earlier address-range checks: the
retained prefetch range has already passed them, while a successful refill
sets its start to the current instruction address. A nonzero-count check is
retained. No bus read, fetch length, exception, preview operation, guest clock,
public interface or persistent state is deliberately changed. An independent
12,800-case model checked the range implication for small addresses, empty
and nonempty buffers, mismatched expectation, and refill success/failure; it
does not prove real CPU execution, callbacks, memory-lock or complete guest
equivalence.

The configured-CMOS fixed-work baseline/candidate/candidate/baseline samples
at 10,000,000 instructions, 79,964,294 guest clocks and PC `0003dbc5` were
21039/20035/18726/20360 ms on x64 and 23733/23850/23892/23813 ms on x86.
Both x64 pairs favored the candidate; x86 differences were below one percent
in this single sequence. Host drift, startup-only input and matching endpoints
do not establish reproducible gameplay speedup or complete state equivalence.
Scratch comparison log SHA-256:
`4BF740CA09D7DB7544FD02A88574F681EED594DB85FF9A100F7C113C806295DE`.

The existing CPU prefetch test is extended with actual 8088 and 486 guest
execution to confirm stale same-range bytes are discarded when the expected
PC changes and a valid-but-empty buffer refills. This tests observable behavior
rather than the implementation's number of internal calls. The actual Shared
diff is +6/-8 source lines, +27 test lines and two manifest hash updates.

The existing prefetch, decode, cache and LOCK-focused tests passed 7/7 per
width. The cached receiver passed 712/712 unit and 48/48 default integration
tests per width, plus 5160, 5170 and DeskPro profile integrations at 3/3,
5/5 and 6/6 per width. MyRaidenII's selected-set startup and product Console
checks passed per width; both build trees were restored to the default profile.
All fourteen deployed EXEs passed PE-width verification and are listed in
the [artifact inventory](t582-s34-prefetch-artifacts.json). NXVM, MyPC110,
MyNES and MyRaidenII deployment hashes match their current build outputs;
MyNES was byte-identical and both MyRaidenII receivers were updated. Other PC
profile deployment identities were checked in their profile integrations.

No real-time gameplay, native RDP interaction or physical-device session was
verified. These tests qualify the bounded consolidation, not the remaining
PC110 performance objective.

SHA-256 (qualification log, external receiver log, artifact inventory):
`54F03DACDEA980E2E1578EB2580A9E0C2DE2555CF1779AF166037D1C29A38552`,
`D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`,
`C2FE481B85776EF9F972B49FAB74FA919FFC53B64AD24C85041D64A91EB363F8`.
