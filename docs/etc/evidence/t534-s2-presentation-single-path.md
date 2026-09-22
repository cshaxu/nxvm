# T534 S2 Presentation Single Path

Core display state is captured once, converted by the Machine adapter into the
Common copied frame ABI, and published only by Common Machine to Common UI.
The former Core guest-presentation mailbox duplicated that frame, its lifetime
and its test observation route; it is removed with its source, interface,
target and smoke.

The adapter conversion now stages into a short-lived heap value and assigns the
destination only after all validation and copying succeeds. An unsupported
source therefore leaves the previous destination unchanged.

Integration probes retain their legacy assertion shape through a test-only
adapter that captures the real Common frame. It owns no mailbox and cannot
publish a frame.

Verification: focused frame/display tests pass; repository-only unit is
333/333 passing; external integration is 20/20 passing.
