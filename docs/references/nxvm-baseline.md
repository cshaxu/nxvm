# NXVM Reference And Import Baseline

NXVM is the formal machine foundation for ntvdm64. Its eligible CPU, memory,
BIOS, device, debugger, assembler, and disassembler code may be imported under
the policy in [../source-policy.md](../source-policy.md).

The sibling checkout remains read-only. Before importing any unit, pin its
commit, record source and destination paths in `docs/provenance/`, preserve
copyright notices, record the copyright-holder MIT authorization, describe
modifications, and add focused tests.

NXVM's documented quick-and-dirty areas remain Supporting evidence for behavior
until tests or stronger sources validate them; they are not a reason to discard
NXVM as the implementation foundation.
