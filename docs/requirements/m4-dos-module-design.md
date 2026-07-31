# M4 DOS Module Design Requirements

M4 is a design milestone for the M5 bounded DOS profile. It turns the M5
feature list into a testable ABI against the M3 Machine/DOS contract.

The specification must fix COM load segment and image limits; PSP, environment,
DTA, stack, and initial registers; `INT 20h` and `INT 21h` entry/return rules;
per-service register and FLAGS effects; memory-fault behavior; standard and
fixture-file handle allocation; read, seek, EOF, and error behavior; pathname
grammar; deterministic input-blocked stop/resume behavior; and exit reporting.

Every allowed service requires a table row and an executable test vector. The
design must state which behavior is intentionally absent, rather than treating
test implementation as the specification. M4 closes only when its vectors can
be consumed unchanged by M5.
