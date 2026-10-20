# T3 S27: Selected Control No-Op Span

## Boundary

S27 changes only the MyRaidenII Board and its existing Board-bus unit. It
does not change a Shared component, public interface, CPU decoder, COP, Video,
ROM archive, or published executable.

## Evidence And Model

The source-grade MAME Raiden II main-CPU map leaves `0x6c8--0x6c9` unassigned
between its named `0x6c6--0x6c7` destination and `0x6ca--0x6cb` receiver.
The selected program reaches the existing V30 memory provider with a word
store beginning at `0x6c8`. The provider receives its two byte lanes at
`0x6c8` and `0x6c9`; this is an access-shape observation only and retains no
guest payload, registers, program counter, instruction bytes, or trace.

The Board therefore owns only this exact write-only L2 span:

- one- or two-byte writes wholly inside `0x6c8--0x6c9` complete without state
  or readback;
- reads and inspection retain the existing `INVALID_STATE` refusal;
- neighbouring `0x6ca` and all other unknown control addresses retain their
  existing receiver or explicit refusal.

The pre-existing `0x43e--0x43f` selected write-only hole uses the same exact
owner. This is not RAM, a generic fallback, a COP model, or a claim that the
unmapped reference entry has hardware-register semantics.

## Verification

The existing Board-bus executable on x64 and x86 proves byte/word write
admission, no readback, the real V30 word-store path, and the unchanged
neighbouring unknown refusal.

The opt-in selected-set probe advances beyond `0x6c8` on both widths and
stops at the next bounded receiver:

| Width | Stop | Fault code | Executed | Ticks |
| --- | --- | ---: | ---: | ---: |
| x64 | internal CPU case fault | `0x6ce` | 65,624 | 1,050,432 |
| x86 | internal CPU case fault | `0x6ce` | 65,624 | 1,050,432 |

`0x6ce--0x6cf` is the next named sprite-flags receiver in the reference map.
S27 deliberately does not turn it into another no-op: its Video/Sprite owner,
direction and behavior require their own receiving task. No boot, rendering,
sound, desktop, RDP, or gameplay qualification is claimed.

## Cleanup And Qualification

The transient selected-set staging lanes and Debug scripts were deleted with
the repository cleanup tool after both probes ended. S27 has no runnable
product target, so it does not publish an executable. The complete
repository-only unit suite passed serially on both widths: x64 `687/687` in
377.29 seconds and x86 `687/687` in 448.17 seconds. Serial execution is
required by the existing Shared selftest fixture-root collision documented in
the MyRaidenII TODO, not by this Board change. The restricted sandbox cannot
create the cleanup test's intentional junction; the qualifying runs executed
outside that restriction and passed that safety test on both widths.
