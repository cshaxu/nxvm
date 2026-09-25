# M6 T43 S8 Receiving Artifacts

MyNES-hosted Shared S8 migrates actual predicates/copied boolean fields and
matching tests, removes dead references and flattens small duplicate paths.
Shared implementation is pinned at bfcbd31b2 (S8 P1).
The eight NXVM receivers are rebuilt together; mixing pre-S8 frame layouts
with this source is unsupported. No NXVM product-source change is needed.

Complete repository-only unit suites pass: x64 335/335 (31.83 seconds), x86
335/335 (70.81 seconds). Four fixed-profile vm-0-5-0535 targets were built in
both optimized Release widths. PE architecture and absence of debug sections
were verified with PE headers/objdump; identities remain 0.5.0535. No new
external-ROM integration or manual boot acceptance is claimed.

| Profile / architecture | Bytes | SHA-256 |
| --- | ---: | --- |
| default / x64 | 1248588 | 7DEB4B2AC4A19EFDF3A8FA514099D571D04295D328A833F356A87D17CD4CB4DB |
| default / x86 | 1417140 | D67D90A96FFC8AC630A39BE851FA418F7C5FE08AABB94FC8D7C3351E837553E2 |
| XT / x64 | 1248588 | F69336CEE60D84CC9C072504A58E385F3730569384BBE1C8C7BE6337895541E6 |
| XT / x86 | 1417140 | 74E2D90DCBCAA5508AC2E35E289092931A9AA69A694DACA4B35108603A562996 |
| AT / x64 | 1249100 | 8142F7C7E96B29A3628418323F615EC2188429E789EF8821727FB6C92AAF218B |
| AT / x86 | 1417652 | D54E9B7428E9042CD3223A879000BA10CB55A36A02F87CE8901CDB686BFCE24F |
| Model 40 / x64 | 1249100 | 7E95A07D4FA1B8DE867C1F5C7D49F132BEF558448061733B9DF4829A782F102D |
| Model 40 / x86 | 1417652 | F9F5C32ABCEE6C5184D288B2E2B14B9FEDDCCE8D33ACA7C4BCE3F7CD322B0B16 |

Outputs remain under assets/nxvm/<profile> with existing filenames. Reusable
t41-s8-nxvm build trees are restored to default selection. The owner's default
NXVM.ini edit console_control=0 fails the current true/false-only parser and
remains pending an explicit spelling/grammar decision, not a validated change.
No protected BYOB inputs or machine-local paths are committed.

[Shared audit and repair evidence](../../../mynes/etc/evidence/m6-t43-s8-six-corpus-quality-audit.md)
owns the finite batch and original-style/ownership review. T43 stays open.
