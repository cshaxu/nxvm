# T7 S65 Residual COP L1 Compatibility

## Decision

This packet admits the three previously audited residual COP command forms as
one private L1 compatibility family. Each form uses the existing command
admission, pending-operation and completion path, then completes without a
modelled result, register update, memory transfer or image lookup.

This is deliberately not an algorithm claim. Available external emulator
implementations were reviewed as L2 behavior evidence and leave the same
forms unimplemented; no primary description establishes operands, outputs,
timing, or a failure/result contract. No external source, table or media was
imported or translated.

Unknown command forms remain rejected. The ordinary command-completion status
bit handling already owned by the dispatcher is retained; the new family adds
no additional observable state mutation.

## Implementation And Coverage

`src/myraidenii/core/cop.c` adds one private classifier kind and one empty
completion branch. No public header, Shared component, queue, thread, state
object or second command path changes.

The existing Board smoke executable now proves, for every admitted residual
form:

- normal two-byte admission and pending rejection before completion;
- deferred success through the existing COP advance owner;
- preservation of existing writable COP address fields and ordinary RAM;
- reset followed by a fresh successful admission; and
- unchanged generic-unknown rejection through the existing admission test.

## Verification

- Focused `myraidenii.core/board-bus-smoke`: passed on x64 and x86.
- Complete `myraidenii` unit label: 12/12 passed on x64 and x86.
- Existing selected-set wrapper: one Coin/Start frame-sequence run per width
  completed its fixed 100,000,000-instruction budget with the same neutral
  completion category and no COP provider failure. The wrapper-owned external
  staging child was removed after each run.

This verifies bounded continued execution and the existing input delivery
route. It does not claim title-screen fidelity, gameplay, real residual-COP
semantics, audio output, desktop/RDP validation, or release qualification.
