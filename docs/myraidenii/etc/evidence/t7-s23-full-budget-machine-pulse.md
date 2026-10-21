# M1 T7 S23 — Full-Budget Machine Pulse

## Scope and grade

S23 reuses the existing selected-set startup Probe, ROM inventory wrapper,
Machine driver and Board control owner. It makes no source, test, API, ROM,
media or Shared change. The final bounded run is **L2** runtime evidence; the
20,000,000-instruction limit is **L1** containment only.

The existing wrapper ran the Machine-driver `5`/`1` Coin 1/Start 1 pulse at
5,000,000 and 10,000,000 instructions. A temporary one-time OS task merely
allowed the unchanged Probe to outlive the terminal's 30-second observation
limit. Both tasks, output logs and wrapper-generated staging directories were
removed after the comparison.

## Result

The x64 and x86 output records were byte-identical:

| Field | Both host widths |
| --- | --- |
| Input route and mode | `machine-driver`, pulse |
| Press / release | 5,000,000 / 10,000,000 instructions |
| Executed / elapsed ticks | 20,000,000 / 324,372,782 |
| Stop/fault | budget; no CPU, COP or Board-control fault |
| Final frame | 320x240, 74,865 non-black pixels, `baabe5eae2033206` |
| Frame samples / transitions | 4 / 3 |
| Sound command and interrupt at all samples | `0x00`, `0x00`, not pending |
| Copied PCM / nonzero frames | 4,096 / 0 |
| Dropped PCM frames | 969,022 |

The four copied frame samples match the existing 20M held-input observation:
`d7e4cff81e7dabcd`, `e6d49832ee1363b8`, `2b271d19a52864f7`, and
`baabe5eae2033206`. The pulse therefore neither introduces a host-width
divergence nor exposes a new fault. It also does not provide a title, credit,
start, gameplay or audible-output semantic signal.

## Boundary

This is stronger only in duration and input routing than S21. It proves that
the existing finite electrical pulse survives a full permitted run through the
same Machine driver. It does not prove that the guest accepts it: neither
frame-digest change nor elapsed runtime is a lawful guest-state observer.
COPX-D2 remains the earliest known unconsumed Board input and remains blocked
until a finite request/index/result contract is independently established.

## Verification

- Confirmed the reusable x64/x86 Probe targets were already current.
- Ran the existing validated wrapper once per width with its 20M maximum and
  `--coin-start-pulse`.
- Compared the complete neutral output records byte-for-byte.
- Confirmed no Probe process or staging directory remained, then removed the
  two one-time task registrations and four temporary log files.
- This packet has no production or test delta; no unit rebuild was required.
