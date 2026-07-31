# M5 T6 S1 Console Command Adapter Verification

GCC built and ran `nxvm-product-console-smoke` under `-Werror`; it emitted
`M5:T6:S1:NXVM-CONSOLE:OK`. The smoke covers case-insensitive parsing, invalid
input rejection, media mutation allowed only before start, start/stop/debug/
resume transitions, and clean exit. The adapter invokes a typed operation
callback and contains no baseline-global or host-UI access.
