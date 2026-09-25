# M6 T43 S8 Receiving Artifacts

MyNES-hosted Shared S8 migrates actual predicates/copied boolean fields and
matching tests, removes dead references and flattens small duplicate paths.
Shared implementation is pinned at bfcbd31b2 (S8 P1).
The eight NXVM receivers are rebuilt together; mixing pre-S8 frame layouts
with this source is unsupported. The subsequent owner-approved S8 P4 correction
also changes the existing INI parser to accept only 0/1 boolean values.

After that correction, complete repository-only unit suites pass: x64 335/335
(22.70 seconds), x86 335/335 (24.00 seconds). Four fixed-profile vm-0-5-0535 targets were built in
both optimized Release widths. PE architecture and absence of debug sections
were verified with PE headers/objdump; identities remain 0.5.0535. No new
external-ROM integration or manual boot acceptance is claimed.

| Profile / architecture | Bytes | SHA-256 |
| --- | ---: | --- |
| default / x64 | 1248588 | 26877BDB465DD4AC145AF64CD4B6045BDF8EF50888E44B7F10816BD468EBE6D2 |
| default / x86 | 1416628 | 3826F618137B280281BCB7EAFC9F96E19B646E95C77BBEF115ED34B6767C9672 |
| XT / x64 | 1248588 | 099583C504ED3A0DFA156CF9A105C5E51C7405318857167F0C0DFDF52185FEC6 |
| XT / x86 | 1416628 | 4D7BAC31D53DD4F21DFF066D40844A2665F635D59DA794577575A9C1A9AA2A9C |
| AT / x64 | 1249100 | 8571DD6336348F4EF3A703C6F45B1A0D1A75C7EB495E901230C3812A60B2063D |
| AT / x86 | 1417140 | 56BE35550B10BAA4E361EB01E2AE482079EAF2427105DDC599D34DDD7CC15404 |
| Model 40 / x64 | 1249100 | 22CF40637676A49BFDA6B94FA9B9CF0D3C7FA55F210694302B6C5DFC738F68CC |
| Model 40 / x86 | 1417652 | 04B6208411B99629668799DCB8AE66DA0B3B6769905BA39DECCEC28D84C2AE69 |

Outputs remain under assets/nxvm/<profile> with existing filenames. Reusable
t41-s8-nxvm build trees are restored to default selection. All four supplied
NXVM INIs now use console_control=0; the owner's existing default edit is
preserved. A table-driven repository-only regression accepts 0/1 and rejects
true/false, uppercase variants, 2, -1, 01, empty, yes and on, with cleared
failure output. No compatibility parser or alternate loading path is added.
Source/test delta is +17/-3 (net +14), counted with git diff --numstat over
ini.c and nxvm_ini_smoke.c; the added lines are the rejection matrix.
The sweep used git ls-files '*.ini' and rg -n console_control across product,
tests, assets, tools and cmake. NXVM has this one boolean INI key; MyNES's sole
INI defines rom/display, not boolean values, so requires no modification.
No protected BYOB inputs or machine-local paths are committed.

[Shared audit and repair evidence](../../../mynes/etc/evidence/m6-t43-s8-six-corpus-quality-audit.md)
owns the finite batch and original-style/ownership review. T43 stays open.
