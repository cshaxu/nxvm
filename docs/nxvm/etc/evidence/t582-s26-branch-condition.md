# T582 S26 — branch timing condition calculation

The timing evaluator computed all five arithmetic condition flags before
choosing a branch condition. It now inspects only the flags needed by the
selected condition. This is a single existing private timing function: no new
state, interface, execution path, guest clock or timing formula. Production
source is +18/-27 lines, net -9.

A task-only C comparison tested all 32 combinations of the five relevant
flags against all 256 condition-byte values and null-output behavior on x86
and x64. The existing CPU instruction test additionally checks 16 conditions,
32 flag combinations, four instruction forms and both operand sizes through
actual instruction execution; no duplicate test suite was added.

With private copied PC110 media, the fixed 10,000,000-instruction startup
checkpoint produced 79,822,202 ticks and PC `00039119` in all runs. Ordered
baseline/candidate/candidate/baseline times in milliseconds were
20,998/21,479/21,756/21,665 on x64 and
24,114/24,514/25,302/27,842 on x86. Run-to-run variation prevents a startup
speedup claim.

A separate synthetic `CMP`/taken-`JE` loop ran 10,000,000 instructions in
the existing CPU fixture and neutral Machine. All Machine runs reported
40,000,014 ticks and PC `000f0002`. In the same ordering, CPU/Machine times
in milliseconds were 4919/11841, 4867/10596, 4439/9766 and 4512/10659 on
x64; and 6830/14969, 6921/14832, 6839/13645 and 6534/14561 on x86. The
Machine samples favor the candidate in this synthetic workload; CPU samples,
execution order and host variance do not establish a stable isolated speedup.
This loop exaggerates branch frequency and is not an actual Game benchmark.
Matching checkpoints are not proof of complete machine-state equivalence.

Cached x86 and x64 unit suites each passed all 712 tests. Default integration
passed all 48 tests on each width, and the selected 5160, 5170 and DeskPro
integration suites passed 3, 5 and 6 tests respectively on each width. The
MyRaidenII external startup and product-console probes passed on each width;
they do not establish gameplay. All 14 executables were built or confirmed
current; PE width and hashes are recorded in
[the artifact inventory](t582-s26-branch-condition-artifacts.json). MyNES did
not change. Both cached build trees were restored to the default PC profile.

A subsequent x64 private-media product-path probe ran the actual guest to
Personaware and sent the existing Machine input sequence. It reached the
complete Game scoreboard 25 one-second samples after Enter; the complete
run exited successfully in 265.48 seconds. The saved frame was visually
reviewed and has SHA-256
`D6F653BE3BDA22405B44FA156BD2D5D84B6A63E19BC12778208954B83C07DE18`.
Earlier direct probes reached the checkpoint in approximately 27–33 seconds,
but these sequential runs do not begin in demonstrably identical complete
guest states and cannot establish a precise improvement. Input was injected at
Machine, not native Window/RDP, and no gameplay session was exercised.

PC110 real-time execution and native Window/RDP gameplay remain unproven;
S26 and T582 remain active.
