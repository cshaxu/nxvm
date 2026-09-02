# M5 T514: Win32 Unicode Guest-Keyboard Ingress

T514 converges Windows Console, Window and Remote Desktop soft-keyboard input
through one Core-platform physical-key normalization boundary. Its retained
[proposal](M5-T514-win32-unicode-guest-keyboard-ingress-proposal.md) defines
the completed outcome.

## Accepted Progress

| Subtask | Result | Implementation P |
| --- | --- | --- |
| S1 | Frozen the Windows Console/Window/RDP packet ledger, source basis and one-normalizer replacement boundary; unit and documentation governance pass. | `196bfb79` |
| S2 | Replaced scan-only ingress with one Core-platform physical-key path; added virtual-key and UTF-16 recovery, VM-path proof, stripped 0514 build, and owner-verified RDP soft-keyboard input. | `93324876` |
| S3 | Moved duplicate-character correlation from Window into the sole Core-platform normalizer and completed its finite packet regression matrix. | `3bb85fb4` |
| S4 | Recorded successful owner RDP observation and task gates: unit 317/317, integration 40/40, governance and stripped 0514 artifact. | `5de5cd91` |

## Closure

T514 closes with one Core-platform Win32 normalizer for physical, virtual-key
and Unicode recovery. It alone owns UTF-16 and duplicate-character correlation;
Console and Window are thin adapters. VM binds the input sink and retains only
its product F9 policy, while Core machine remains the sole KBC and guest-state
owner. The owner-confirmed RDP result, full gates, artifact hash and source/test
accounting are retained in the [S4 closure evidence](../etc/evidence/t514-s4-win32-rdp-frontend-closure.md).
