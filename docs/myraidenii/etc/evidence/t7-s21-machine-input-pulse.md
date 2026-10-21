# M1 T7 S21 — Machine-Driver Selected-Set Input Pulse

## Scope and grade

S21 changes only the existing selected-set integration Probe.  Its optional
Coin 1 / Start 1 held and pulse modes previously called the Board electrical
control API directly.  They now create the existing MyRaidenII Machine adapter
without a host PCM stream, obtain its one existing Emulator driver and submit
ordinary KVM key events for `5` and `1` through `deliver_input`.

The Board stays the sole control-polarity owner.  The Probe retains its
existing one Core execution loop, sampled output and bounded runtime images;
it adds no Product/Core behavior, public API, queue, desktop automation or
second executor.  The selected-ROM observation is **L2** runtime evidence.
The 2,000,000-instruction limit is **L1 containment**, not a timing or
gameplay claim.

## Result

The owner-supplied selected set ran once per host width in pulse mode.  Both
runs report `input-route=machine-driver`, the same 500,000 / 1,000,000
press/release thresholds, and identical finite output:

| Field | x64 | x86 |
| --- | ---: | ---: |
| Executed / ticks | 2,000,000 / 33,301,143 | same |
| Pulse transitions | 1 / 1 | same |
| Final frame digest / nonblack | `a06b9c216e1ce034` / 74,795 | same |
| Frame transitions | 2 | same |
| Sound command bytes at every sample | `0x00`, `0x00` | same |
| Sound interrupt pending at every sample | no | no |
| Copied PCM / nonzero frames | 4,096 / 0 | same |

No transient stage remains under the external diagnostics root after either
run.  No ROM byte, screenshot, guest trace or staged image was retained.

## Disposition

This proves the selected Board can receive the finite diagnostic edge through
the existing Machine KVM ingress rather than the Probe directly invoking Board
controls.  It does **not** prove that the Product Window has focus, that a
desktop/RDP keyboard event reaches KVM, that the title accepts the action, or
that gameplay, graphics fidelity or audible output is working.

## Verification

- Rebuilt `myraidenii-selected-set-startup-probe` in the reusable x64 and x86
  Ninja trees.
- Ran the owner-ROM wrapper once per width with `--coin-start-pulse` at its
  existing 2,000,000-instruction default.
- Ran all 12 registered MyRaidenII unit tests per width: all passed.
- Ran documentation governance and `git diff --check`.
