# PC App test adapter encapsulation

Candidate, not implementation admission. Review the four PC App test trees'
direct use of `x86-nxvm/machine/machine_private.h`. Classify each use as
hardware observation, component-owned proof, true App integration or fault
injection; relocate lower-owner assertions only where their context permits,
and use existing public operations for App-owned assertions where appropriate.
Private-header use alone does not prove an individual test is misplaced.

Exit: every use has an explicit owner and disposition, assertions and relevant
four-profile coverage remain intact, and affected tests pass on both widths.
No production raw-Core getter, copied private layout, weakened assertion or
new compatibility path. Shared changes require appropriate authorization.
