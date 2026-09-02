# M5 T514: Win32 Unicode Guest-Keyboard Ingress

T514 converges Windows Console, Window and Remote Desktop soft-keyboard input
through one Core-platform physical-key normalization boundary. The admitted
[proposal](../proposals/m5-win32-unicode-guest-keyboard-ingress.md) defines
the complete outcome; `CURRENT.md` owns the active S contract.

## Accepted Progress

| Subtask | Result | Implementation P |
| --- | --- | --- |
| S1 | Frozen the Windows Console/Window/RDP packet ledger, source basis and one-normalizer replacement boundary; unit and documentation governance pass. | `196bfb79` |
| S2 | Replaced scan-only ingress with one Core-platform physical-key path; added virtual-key and UTF-16 recovery, VM-path proof, stripped 0514 build, and owner-verified RDP soft-keyboard input. | `93324876` |
