# NTVDMx64 Behavioral Reference Boundary

NTVDMx64 demonstrates that DOS applications can be integrated with modern
Windows filesystem, console, input, and device services. It is useful for
defining user-facing compatibility goals and for identifying edge cases in
DPMI, graphics, mouse, and host redirection.

This project does not reuse its implementation. In particular, it excludes
Microsoft-internal NTVDM restoration, undocumented loader patching, symbols,
injection, registry takeover, and code copying. A behavior observation must be
captured as an independent probe or evidence record before it influences this
runtime.
