# T582 S9: CPU instruction observation investigation

After each successful instruction timing selection, CPU copies completed
instruction and register observations before ExecInt may replace decoder state
for debug trap, NMI, INTR, system management or error entry. Core consumes the
copy for physical-timing eligibility and explicit retirement observers; the
debugger and CPU diagnostic paths also use observation. Simply omitting it
can report the wrong completed instruction, register state, or lexeme inputs.

A task-owned experimental bypass omits the copy entirely, knowingly violates
this contract and is not a production candidate. Both direct-compiled variants
use identical Release flags and copied inputs. At 10M instructions, 79,964,294
guest ticks, PC 0003dbc5, x64 baseline/bypass were 25,636/21,734 ms; x86
23,907/26,940 ms. These single sequential pairs disagree in direction and do
not establish savings; equal PC/ticks do not prove observation correctness.
No production code changed. A future design must preserve observation before
every actual entry which can overwrite instruction/register facts; detecting
that boundary must not duplicate interrupt arbitration, alter debug traps or
introduce a second state owner.

## Board PIT deadline candidate

The board queries each PIT channel and converts each valid positive device
deadline separately. Because the three share one clock domain, the minimum
positive device deadline can be selected first and converted once: the clock
conversion is monotonic for valid input, and an overflowing larger input cannot
become valid when a smaller positive input is invalid. Zero remains a separate
immediate event. A candidate must use a separate found flag rather than
UINT64_MAX as a sentinel, so a valid maximum is not dropped.

A private early-startup comparison using an uncorrected sentinel prototype
ran 10M instructions at identical 79,964,294 ticks and PC 0003dbc5: x64
baseline/candidate 24,610/23,596 ms; x86 30,114/26,812 ms. This single pair
is evidence of potential, not an acceptable implementation or a variance
study. The next S must fix the sentinel, test conversion overflow, zero,
missing/multiple channels, phase and clock ratios, and prove guest behavior.
No production code was changed in S9.

A task-owned mathematical comparison checked 150,000 deterministic synthetic
three-channel cases, including zero, UINT64_MAX, invalid clock domains,
multiplication overflow, phase and an existing deadline. The found-flag
min-before-conversion model matched the prior per-channel aggregation. This
model is a boundary check, not a substitute for the real PIT/board tests.
