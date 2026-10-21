# M1 T7 S36 — Input Window Observation

## Scope

S35 proves that the selected V30 and Sound execution paths consume every
existing Board input owner somewhere in the bounded run.  S36 narrows that
observation without changing production behavior: the existing selected-set
Probe snapshots the already copied read aggregate immediately before each
Coin/Start press and immediately before its release.

It records no input value, guest state, ROM, RAM, trace, screenshot or other
protected payload.  It adds no Board state, API, execution route or input
path.

## Bounded result

Both host widths ran the owner-ROM vblank-aligned sequence at the existing
20,000,000-instruction budget.  Each edge was delivered once at vblanks
300/330/360/390.  The following are read-count deltas while each named key was
asserted:

| Asserted window | DIP | Player | System | Coin |
| --- | ---: | ---: | ---: | ---: |
| Coin (`300..330`) | 0 | 28 | 56 | 98 |
| Start (`360..390`) | 0 | 30 | 60 | 98 |

x64 and x86 agree in every row.  Their final frame digest/nonblack remains
`ccad0e0867e39b9c` / 74,865; the existing audio, Video and frame-sample
summaries also remain identical to S34/S35.

## Disposition

The running selected program reads Player/System/Coin while each input is
actually asserted.  Together with S34's one-time delivery and S35's owner
coverage, this rules out local host timing, driver delivery, Board register
composition and simple "read outside the pulse" explanations for the neutral
bounded frame result.

This still does not expose or prove a guest acceptance decision, accepted
credit, title, gameplay, graphics fidelity or audio fidelity.  The next T7
receiver must obtain one finite non-payload game-state or Board-semantic
checkpoint before changing any hardware model.

## Verification and cleanup

- The existing selected-set Probe rebuilt successfully on the stable x64 and
  x86 Ninja trees.
- The affected MyRaidenII unit label, documentation governance and whitespace
  checks pass on both widths where applicable.
- The external staging root is empty after both owner-ROM runs; no ROM or
  diagnostic payload is retained.
