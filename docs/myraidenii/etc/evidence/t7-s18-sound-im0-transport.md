# M1 T7 S18 — Sound IM0 Transport

## Scope

This packet replaces the locally insufficient IM1-only Sound interrupt gate
with one bounded IM0 arbitration point. It changes only MyRaidenII Core and
its existing Sound smoke; it does not modify Shared components, retain ROM
data, or claim audible output or gameplay qualification.

## Evidence grade

The model is **L2**. The inspected external reference is official MAME source
blob `48899caa324b444bd59e22d38466148d8950a547`,
`src/mame/shared/seibusound.cpp`. Its configuration and interrupt callback
show a main-CPU request on RST18 (`0xdf`) taking priority over an FM/YM RST10
request (`0xd7`), with RST18 cleared when accepted. The local Raiden II map
reference was used only to confirm existing Sound address ownership. No
external implementation, table, ROM data, or protected payload is copied or
retained. This is not a primary-manual PDF rule and therefore cannot be L3.

## Local model

The sole local Sound interrupt acceptance point handles IM0 as follows:

1. A pending Board request selects vector address `0x18` before YM.
2. Otherwise a pending YM request selects vector address `0x10`.
3. The Board latch clears only after its RST18 transition is accepted.
4. Guest writes to existing acknowledgement/service addresses do not
   incorrectly clear the Board request.

The existing 13-tick local interrupt cost remains an L2 local timing model.
There is no second clock, worker, queue, or interrupt path.

## Verification

- Existing `myraidenii.core/sound-smoke` covers IM0 RST18, IM0 RST10,
  Board-over-YM priority and accepted Board clear; it passed on x64 and x86.
- Complete MyRaidenII unit suites passed `12/12` on each width.
- The bounded selected-set startup probe ran once on each width at the existing
  20M-instruction cap. All four samples reported `sound-interrupt-pending=0`,
  identically on both hosts. Frame samples remained equal and copied PCM
  remained all-zero; this is transport observation, not proof of sound or
  gameplay.
- Documentation governance and `git diff --check` passed.

## Remaining boundary

This packet establishes only the local Board/YM interrupt transport model.
PCM synthesis, audible output, title/attract, input, and gameplay remain
separate qualification receivers.
