# T582 S26 — CPU timing result publication

The private CPU timing selector had one caller, which passed the address of
the CPU's own `timing_result` as the output argument. For Intel profiles the
selector wrote the same result to that object twice. The V30 branch wrote it
once through the argument. The selector now publishes directly to the existing
CPU result in both cases; its unused output argument and duplicate write are
removed. No public interface, timing formula, guest clock or execution path
changes. Production code changes are +5/-8 lines, net -3.

The cached x86 and x64 unit suites each passed all 712 tests. The default
integration suites each passed all 48 tests; the 5160, 5170 and DeskPro
profile integration suites passed respectively 3, 5 and 6 tests per width.
The MyRaidenII external selected-set startup and product-console probes passed
on both widths; these startup probes do not establish gameplay. Both stable
build trees were restored to the default PC profile. The 14 product executable
PE widths and hashes are recorded in [the inventory](t582-s26-timing-publication-artifacts.json).
MyNES was rebuilt or confirmed up to date and its unchanged executables retained.

These checks do not establish a measured speedup, RDP behavior or PC110
real-time usability. T582 and S26 remain open for the larger performance goal.
