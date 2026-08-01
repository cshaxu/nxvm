# M5 T10 S4 P1 Execution Context Carrier

The session-owned execution-context carrier is bound and unbound at the
existing device lifecycle points without changing refresh order. `nxvm-m5-t10`
compiled the new source and the finite real CPU probe retained its established
`#UD` and success marker. Controller state remains unchanged in this first step.

P2 binds the context to the existing CPU, RAM, port, and device objects through
opaque slots at `deviceInit`. The project GCC build and finite real CPU probe
passed after this binding; legacy anonymous state typedefs remain unchanged.
