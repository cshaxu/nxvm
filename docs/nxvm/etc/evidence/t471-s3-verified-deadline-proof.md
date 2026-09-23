# T471 S3: Verified Deadline Proof

`M5:T471:S3:VERIFIED-DEADLINE-PROOF:OK`

The existing Model-339 clock-contract fixture remains a Core-only synthetic
proof: it assigns the already tested 8 MHz verified axis only after creating a
normal Model-339 session, and it does not change the descriptor's unavailable
rate.

It programs the source-qualified PIT deadline, then invokes the VM HLT waiting
boundary twice:

| Mode | Required observation | Result |
| --- | --- | --- |
| Standard | Session pacing origin becomes valid before the Core-selected deadline advances. | Passed: elapsed Core time reaches only the selected deadline. |
| Turbo | No pacing origin is established; the same selected deadline advances through the existing Core path. | Passed: elapsed Core time reaches only the selected deadline. |

The fixture also retains its unavailable Model-339/default-PC/AT checks. It is
run directly because it is a targeted contract executable, not an independent
registered current CTest. The current speed-policy CTest continues to prove
that unavailable Standard and Turbo leave Core elapsed ticks unchanged.
