# T382 S8: Corrective Reclosure Audit

`M5:T382:S8:RECLOSURE:OK`

The prior T382 S1--S7 closure audit is retained as historical evidence but is
superseded for final acceptance by this approved S8 corrective review.

| Requirement | Current proof | Result |
| --- | --- | --- |
| Strict 64-bit memory quantity transport | S8 YAML `4294967296` catalog-to-Console-to-provider marker | Accepted. |
| Console narrowing sweep | Option construction formats native `STD_SIZE_T`; no 32-bit cast remains on that path | Accepted. |
| Current artifact baseline | `nxvm_0_5_0382.exe` SHA-256 below | Accepted. |
| Current gate | Single-worker aggregate: 251 starts, zero failures, S8 is a registered current test | Accepted. |
| Previous S1--S7 mechanisms | Retained individual evidence and prior closure audit | Accepted, with S3 corrected by S8. |

The final artifact is
`6507E88A85F9E85ED4EB12D1B0B376DED5894771D178E3D1B2B5B89202BC8D57`.
T382 remains a quality-boundary task only; it does not claim L3 hardware
closure or resolve the separately retained CR-mutation debt.
