# M5 T12 S10 Keyboard Ingress Verification

The retained Win32 KEYUP and FOCUS path now samples host modifier/toggle state
into a copied `nxvm_platform_vm_request`, enqueues it through the synchronized
VM ingress transport, and applies it only from the device execution boundary.
Closed or full ingress returns a non-OK status, so the Win32 producer retains
the original direct state application as its fallback. The KEYDOWN-local status
path remains direct before the unchanged legacy key mapping reads modifier
flags; it is not an ingress producer.

The focused transport smoke binds a keyboard-state consumer, proves copied
delivery at one execution boundary, verifies FIFO drain, and retains the
existing closed/full queue cases. GCC built the user-facing `nxvm-m5-t10.exe`
and focused targets. The following bounded gates passed on the local approved
fixtures:

- transport smoke;
- finite CPU `#UD` probe: `M5:T1:S1:CPU-PROBES:OK`;
- FDD-backed execution-context lifecycle: `M5:T10:S4:CONTEXT-LIFECYCLE:OK`;
- FDD/HDD full-PC reset vector: `M3:T3:S1:FULL-PC-PROFILE:OK`;
- no-media Console `help`, `info`, and `exit`, including the retained banner;
- delayed-input `debug`, `q`, and `exit`, including the retained `-` prompt.

No raw instruction recorder was used. This enables no second producer and
makes no claim about a user-visible input behavior change.

The completed T12 developer artifact is the ignored
`build/output/nxvm-m5_t12.exe`. It retains the `0.4.015d.m5t12` Console banner
and its SHA-256 is
`bc8ea2c70950d06f1c502c8a9131e5636c16686e863c68da44a783d673c0c358`.
