# T582 S26 — compact lexical-preview copy

The existing preview copies `t_cpuins` to preserve the source executor while
`ExecInit` and lexical scanning operate on a local instruction record. The
preview does not execute the instruction dispatch table; its forced code
reads do not record normal data-access entries. `ExecBoundary` resets the
local access count before either read path. The existing preview-mode guards
continue to suppress normal execution side effects. The change copies the
fields before the access array and the trailing instruction fields through
the end of `data`; it omits the access array and the `connect` dispatch table.
No public interface, persistent state, timing formula or fetch path changes.

On a private-media x64 startup probe, instrumentation attributed 971,082
copies of 17,144 bytes and 971,082 copies of 820 bytes to the preview. With
the candidate it observed 972,334 copies of 736 bytes and 972,334 of 820
bytes; smaller copies are not included in that histogram. Instrumentation and
the short stop interval affect counts. This identifies work avoided, not its
exclusive elapsed-time contribution.

The same 10,000,000-instruction, 79,964,294-tick, PC `0003dbc5` checkpoint
gave baseline/candidate times of 19,720/18,551 ms x64 and 23,011/22,735 ms
x86. In reverse order, candidate/baseline were 19,188/19,786 ms and
22,832/23,619 ms. These sequential samples suggest a bounded startup benefit,
not equal complete state, a precise percentage, or real-time usability.

A separate x64 product-path Game probe reached the same complete-frame hash
as the baseline and first captured it at sample 33 rather than sample 34.
Samples are ten seconds apart and guest states differ, so this does not prove
an exact interactive speedup. Log SHA-256:
`D49DCBCE0242A1776C4230E14200C9D313E4E88BC75671D2E9380A00329DD2B2`.

The existing CPU timing-preview and bus-timing tests passed with the candidate
on x64 and x86. A separate scratch build first filled the complete preview
object with `A5` before the selective copy; those same tests passed on both
architectures. This is bounded evidence against reading omitted fields in
covered paths, not proof of all possible callbacks or failure paths.

The production change is one X86 Core source file (+9/-1 lines) and its
manifest (+1/-1); no test source or public interface changed. Actual-diff
review retains the original single preview, fetch, exception and timing paths.
The excluded fields remain inaccessible to the covered lexical paths, which
reset the access count and do not dispatch instructions. Future changes to
preview execution must re-audit this ownership boundary.

The cached Release receiving gate passed on each width: 712/712 unit tests,
48/48 default integration tests, and the My5160 3/3, My5170 5/5 and DeskPro
6/6 selected-profile integrations. Two transient concurrent linker output-open
failures were recovered by relinking the same build trees at reduced
concurrency before the full gate; no failure was counted as a passing test.
MyRaidenII selected-set startup and deployed Console probes passed on both
widths. X86 Core manifest, NXVM documentation governance and diff whitespace
checks passed.

All fourteen current App binaries were built or reused from the cached graph
and checked for PE width; [artifact inventory](t582-s26-artifacts.json) records
their hashes. NXVM and MyPC110 deployed binaries match their final build
outputs; the other PC identities were checked through their profile integration
tests. MyRaidenII was deployed from the matching stripped build outputs; both
MyNES binaries were unchanged. Full qualification, external-probe and artifact
inventory SHA-256 respectively:
`896B82CA61C1432E20F56CE8370C7BA16FD04D57A3D47FBA7312DB72531F9C48`,
`D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`,
`D44FA415D552B6D15E2048586357DBAAF0CF0A56391D1DCD66105A45A7034CD8`.
Native Window/RDP, a physical MIDI endpoint and successful gameplay were not
established by these checks. The T remains open: this bounded optimization
does not demonstrate that PC110 is usable at its intended speed.

Implementation is Shared `a794bfcdd`; PC binaries are NXVM `7e3bd57c8` and
MyRaidenII binaries are `b76c5c71e`.
