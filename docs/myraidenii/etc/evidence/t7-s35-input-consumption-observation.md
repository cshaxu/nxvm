# M1 T7 S35 — Input Consumption Observation

## Scope

S34 proves that the vblank-aligned Coin/Start sequence reaches the one Machine
driver.  This packet asks the next smaller question: whether the running local
V30 and Sound execution paths consume the existing Board input owners.  It
adds only resettable counts at those owners.  The counts contain neither input
values nor ROM, RAM, trace, screenshot or other guest payload.

## One owner path

The Board already owns the main-CPU DIP, Player and System read callback.  Its
existing Sound transport callback is the sole Coin read boundary.  The new
aggregate increments only after a successful non-preview main input read, or
after the existing Sound Coin callback succeeds.  Board reset clears it and a
single copied getter exposes it to the existing selected-set Probe.

The existing Board smoke was extended rather than adding a test binary:

- an actual V30 sequence reads each main input register, while its existing
  direct reads retain the alias/byte-value coverage;
- the existing Sound handshake program performs a real Coin callback without
  changing its prior response behavior;
- reset proves every count returns to zero.

## Bounded selected-set result

Each host width ran the existing 20,000,000-instruction no-input baseline and
the S34 vblank sequence.  Both terminated at the containment budget without a
CPU, COP or Board-control fault; their frame samples and neutral Video
aggregates match their respective S34 results exactly.

| Aggregate | Baseline | Vblank Coin/Start sequence |
| --- | ---: | ---: |
| DIP reads | 2 | 2 |
| Player reads | 1,078 | 1,078 |
| System reads | 2,156 | 2,156 |
| Coin reads | 3,662 | 3,660 |
| Final frame digest / nonblack | `ccad0e0867e39b9c` / 74,865 | `ccad0e0867e39b9c` / 74,865 |
| Video vblanks / CRTC writes | 1,122 / 13,052 | 1,122 / 13,052 |

x64 and x86 produced identical values in every row.  The two-Coin-read delta
is consistent with the already recorded two Sound-loop/guest-tick difference
between the schedules; it is not an input-delivery failure and does not expose
a value or guest decision.

## Disposition

The actual selected program consumes all four local input owner paths.  In
particular, Start is not lost before the System register and Coin is not lost
before the Sound callback.  Together with S34's confirmed press/release edges,
this rules out the local host-to-Board and Board-input-read routes as the
explanation for the neutral bounded frame summary.

It does not prove an accepted credit, title, gameplay, graphics fidelity,
audio fidelity or a missing hardware component.  The next T7 receiver must
still establish a finite, non-payload game-state or Board-semantic discrepancy
before altering COPX, Video, Sound, sprites or protection behavior.

## Verification and cleanup

- The focused Board smoke and complete 12/12 MyRaidenII unit selection passed
  on stable x64 and x86 Ninja trees.
- Probe builds passed on both widths.
- All four external owner-ROM stages were removed; no raw media or diagnostic
  output remains in the owned temporary root.
- `git diff --check` and documentation governance are run before closure.
