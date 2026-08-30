# M5 T512 S7 - 80286 final audit

## Authority and tier decision

The normative corpus is Intel order 210498-005, *80286 and 80287
Programmer's Reference Manual* (1987), with its 1985 companion only for the
Appendix-B `XLAT` omission recorded in the ledger.  Visual review of printed
p. 8-6 confirms that `Clocks`, `mem`, `noj` and `pm`, the base+index+
displacement, odd-word and memory-read wait-state adjustments are direct
manual rules.  Therefore the 771 successful-retirement keys are Manual-L3;
no manual range or external-model L2 selector exists for this profile.

The same visual review confirms the `ENTER` lexical-level range 0--31 and its
formal `LEVEL MOD 32` algorithm.  The existing Core path applies that rule
only at the 80286 boundary and higher, preserving the distinct 80186 byte
behavior.

## Current owner and verification

Core remains the sole decoder, protected-mode state/delivery, retirement and
timing owner.  The 80286 runner directly observes all five partitions:

| partition | observed keys |
| --- | ---: |
| non-control | 303 |
| string/repeat | 129 |
| conditional | 54 |
| transfer, gate and task | 193 |
| system/descriptor | 92 |
| total | 771 |

The current generated result passes `Verify-80286TimingResults.ps1` with
771/771 conforming Manual-L3 keys.  Its runner also reports protected-path,
descriptor-path and task next-instruction-byte coverage.  No unallocated or
L1 successful-retirement row was found.  No CPU behavior is implemented in
VM/profile code.
