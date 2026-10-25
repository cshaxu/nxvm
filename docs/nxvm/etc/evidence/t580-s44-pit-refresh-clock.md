# M5 T580 S44 — PC110 PIT/refresh clock relationship

## Ownership and model

The App now expresses PIT inputs as 1,193,182 / 33,000,000 Core source ticks
and RTC inputs as 32,768 / 33,000,000. Both existing construction forms consume
one private App clock header. Refresh transitions use the L2 nominal interval
of 18 PIT inputs, rounded to 498 Core ticks. The existing Board rational clock
and elapsed-tick refresh mechanisms retain all mutable state.

The absolute PIT frequency is a reference model, not a manual-exact timing
claim. PC110-EMU commit `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`
declares 1,193,182 Hz in its core. Open-Source-PC110 commit
`02ccf2e8e19b298a2cf04c5d430278eb087cc368`, Discovery/Chipset/readme.md,
describes the observed system tick using a 65,536 reload at approximately
1.193 MHz. The nominal refresh relationship is L2 and is checked against the
unchanged selected BIOS's range, not represented as a recovered silicon value.

## Real firmware verification

Both x64 and x86 replay the actual timer test and observe its F000:698F pass
branch. It reports:

```text
PC110:TIMER:passed=1 failed=0 delta=1668 executed=59918870 pc=fffffff0 status=0
```

The BIOS admits 1614..1868; its previously measured 4EFF was rejected. No
firmware byte, register, flag or instruction was replaced. The diagnostic stops
on the actual pass branch, a BIOS error halt, a CPU fault or a finite ceiling.
The eight earlier registered firmware-prefix diagnostics are replaced by this
one stronger timer self-test. It executes their reset, protected memory-test,
C000 video-ROM, LPT and refresh-delay prefixes through the same BIOS route.
The other diagnostic modes retain a manual research purpose; their checkpoint
evidence remains historical. The superseded outer-delay-only mode and its
dedicated observation fields/output are removed.
The external-firmware harness now lives in the App integration directory and
is registered as `mypc110.integration/real-firmware-timer`, matching its actual
asset and execution requirements.

The exploratory sampler initially repeated output after a halted guest stopped
retiring instructions. That owned process was terminated and the sampler was
removed. The retained diagnostic now terminates on a POST CLI/HLT, including
its older manual modes, instead of repeatedly advancing device deadlines while
its instruction containment stays unchanged. The timer mode also rejects any
unexpected waiting boundary directly.

## Regression and review

The final `mypc110.integration/real-firmware-timer` CTest passes on x64 in
141.06 seconds and x86 in 169.43 seconds. Both final logs report delta 1668,
pass=1, failed=0 and 59,918,870 retirements. The explicit CLI/HLT containment
and final integration path are present in those tested binaries.

The stable x86 graph built all unit binaries and passed 712/712 repository-only
unit tests, including Shared, all five retained Apps, MyPC110 and MyRaidenII.
The x64 graph first passed 663/667 with four static-check timeouts under build
competition; the subsequent 60/60 selection passed those four checks, all 45
newly enabled MyNES units and all 11 final MyPC110 units. Thus all 712 current
x64 unit cases have passing evidence, without repeating unchanged CPU cases.
MyNES had previously been disabled in both local caches; it is now enabled in
the same stable build trees. No tracked preset or MyNES file changed.

Actual rename-aware `git diff --numstat -- src test`, including the private
clock header, counts 100 additions and 119 removals, net -19 across six paths.
The private header owns constants only. No production state, API or Shared
component changes. One existing Core timeline, Board clock domains and PIT
remain the execution/time owners. The removed outer-delay fields and repeated
test registrations have no remaining caller; the remaining manual modes have
distinct observation purposes and are not additional qualification gates.

Both TFT/DSTN preview pairs rebuilt in the stable Release graphs; their PE
machine values are 8664 for x64 and 014C for x86. They remain prepublication
construction artifacts. No owner INI, external ROM/font/media or existing
published App pair changed. This timer pass does not qualify later POST,
display, Setup, storage or boot; desktop/RDP verification was not performed.
