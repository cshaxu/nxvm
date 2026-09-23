# T504 S4 Mode And Lifecycle Verification

## Actual reachable behaviour

The supported profiles program or retain a PIT source event while waiting. A
real D4 refresh or ATA command therefore coexists with a published future PIT
deadline. Core correctly reports `DEADLINE`, not `L1_COMPATIBILITY`, and both
Standard and Turbo use the existing Core deadline path. This is the required
precedence result; an integration fixture must not disable the real PIT merely
to manufacture an L1 route.

The bounded compatibility action remains covered by the Core-only D4 fixture,
where the fixture deliberately models its source-less L1 condition. It proves
the action uses the normal owner transition. It is not evidence that a current
profile should bypass its sourced PIT deadline.

## Mode and lifecycle boundary

`vm_session_waiting_advance()` returns without calling Core when the session is
not running, when the run result is not an interrupt wait, when Standard sees
an L1 disposition, or when Turbo sees any other disposition. Only the final
Turbo-plus-copied-L1 branch requests the bounded Core action. Reset, stop,
pause/debug control, guest input, interrupts, immediate work, and deadlines
remain outside that branch and retain their existing owners.

The existing speed-policy smoke exercises Standard and Turbo against a real
Core deadline; Model-40 D4 compatibility smoke verifies the real board/PIT
route; the Core time smoke verifies the source-less L1 transition. Together
they establish that a sourced event takes priority and that no host time,
VM-supplied tick count, or controller-specific VM path was added.

S5 must exercise the selected external ROM/DOS workloads and record whether
the runtime uses deadline progress alone or reaches an explicitly eligible L1
condition. It must not claim that the latter was observed unless it actually
is.
