# T582 S26 — generic timing-input collection upper bound

The shared timing selector collects generic formula-input flags and may
capture the next lexical observation. A task-only ablation omitted that call
while retaining the timing evaluator. This deliberately loses public timing
metadata and potentially the next-lexeme observation; it is not an acceptable
product implementation. It estimates whether eliminating the whole collector
could materially improve this particular startup workload.

At the private-media 10,000,000-instruction checkpoint, ordered
baseline/candidate/candidate/baseline elapsed times were
18,973/18,630/18,898/23,783 ms on x64 and
27,496/25,636/25,275/25,149 ms on x86. Each reported 79,964,294 guest ticks
and PC `0003dbc5`. Host variation is large, and the equal endpoint does not
establish state or observation equivalence. These samples do not establish a
reliable improvement or characterize real Game or World Clock throughput.
They cannot justify dropping metadata or introducing a second timing path.

No production code, public API, tests, released binaries or owner media were
changed. This bounded negative result does not close the PC110 usability goal;
future work should address a larger cost or use a more controlled workload.
