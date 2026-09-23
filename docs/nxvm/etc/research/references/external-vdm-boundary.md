# External VDM Behavioral Reference Boundary

External VDM implementations may provide research observations about DOS
application integration with modern Windows filesystem, console, input, and
device services. They can identify edge cases in DPMI, graphics, mouse, and
host redirection.

This project does not reuse their implementation. It excludes undocumented
loader patching, symbols, injection, registry takeover, and code copying. A
behavior observation must be captured as an independent probe or evidence
record before it influences this runtime.
