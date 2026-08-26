# T469 S5: VM Waiting Ownership

`M5:T469:S5:VM-WAITING-OWNERSHIP:OK`

The deleted route comprised the public `virtual_time_source` session config,
session-owned callback state, Model-339 8 MHz source installation, platform
tick accumulator, `vm_session_virtual_time_on_waiting()` and its two smokes.
It allowed a host callback to select a tick count for
`core_machine_advance_time()`.

The sole retained waiting path is `vm_session_waiting_advance()`: it reads the
copied S4 pacing contract and, only if static qualification is available, asks
Core to run `core_machine_advance_to_next_deadline()`. Core then chooses the
dynamic deadline. Current profiles are all unavailable under T388, so Standard
and Turbo both take the existing L2 HLT load-backoff without advancing guest
time. The speed command and its single session state remain unchanged.

`virtual_time.c` is retained only for display's host-millisecond observation;
it contains no guest tick/source state. T388 and T447 static gates now assert
the new Core-deadline/contract boundary. Focused speed, Model-339 and Core-time
smokes plus documentation governance pass. S6 must run the full integration
audit and release build.
