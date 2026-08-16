# T382 S4: FDC Diagnostic Lifetime

`M5:T382:S4:FDC-DIAGNOSTIC-LIFETIME:OK`

The FDC topology-port fixture copies its phase diagnostic while the machine
and FDC owner remain live, then destroys the machine and media registry before
emitting any failure report. The media-change fixture's teardown report reads
only its independent failure step.

Focused topology and media-change smokes pass. The retained failure message
keeps the same deterministic fields without dereferencing destroyed storage.
