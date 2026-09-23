# M5 T472 L2 Profile-Proportional Pacing

T472 removes the fixed Standard-mode HLT poll. It admits a profile-selected,
immutable L2 macro pacing rate only where documented configuration evidence
exists; Core remains the sole guest-time owner. Its final artifact and
verification are recorded at closure.

The selected Rev-3 Model-339 now supplies an 8,000,000 tick/s L2 macro rate.
Standard limits only Core time already completed against the host monotonic
budget; Turbo skips that wait. Default PC/AT and Model-40 remain unqualified
and therefore have no host pacing rather than the removed fixed HLT sleep.
The physical-timebase TODO remains open.

The stripped Release target `vm-0-5-0472`, banner `0.5.0472`, is retained as
`build/output/nxvm_0_5_0472.exe`, SHA-256
`C2B9B95B8A124B96106369E4D88D592A6A7A090D16401C176E93B91349E28BDE`.
