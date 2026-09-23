# T512 S6 P3: 80186 JCXZ and LOOP Manual Correction

Intel *iAPX 86, 88, 186 and 188 User's Manual* (1985), order no. 210912-001,
Table 2-9, PDF page 183 / printed page 2-17, gives these outcomes:

| form | taken | not taken |
| --- | ---: | ---: |
| `JCXZ` | 16 | 5 |
| `LOOP` | 16 | 5 |
| `LOOPE` and `LOOPNE` | 16 | 6 |

The SHA-256 of the owner-managed original is
`2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF`.
The page was visually verified, rather than relying on OCR.

The sole existing control-stack outcome selector had transcribed the two
`JCXZ`/`LOOP` taken values as 15. It now uses 16. `LOOPE` and `LOOPNE` were
already correct. No profile-side path, opcode-local timing path, or additional
state is introduced.
