# T591 S2 Intel 8254 read-back counter selection

[S1 original-ROM traces](t591-s1-ob300-original-rom.md) isolate the BIOS
`1100h` diagnostic to the active-low interpretation of the 8254 counter
selectors. [Intel 8254 data sheet, Figures 10 and 12](https://www.cs.umb.edu/cs341/Intel8254/I8254PIT.pdf)
assign D3:D1 as active-high selects and keep D5/D4 as active-low commands
to latch count and status. The fix changes the select predicate only.

The existing chip read-back test now uses the documented masks for individual,
paired and repeated latches. Its final scenario checks no selection (`E0h`)
against three different live counts, then checks all selections (`EEh`)
against the three status bytes. The 8253 test now uses `C2h` so that either
unexpected status or count latching can violate the fixture. The MyOB300
board test selects counter zero with `E2h` through port `43h` rather than
the inverted legacy encoding. Full unit replay found that the shared
board-level PIT/IRQ0 fixture also used inverse masks through port `0043h`.
Its read-back uses `E4h` for counter one and `D6h` for counters zero and one.
Its counts, waveform and IRQ assertions are unchanged. The separate ordinary
counter-latch and waveform fixtures remain applicable without changes.
No ABI, configuration or second device path is introduced.

| Scope | Added | Removed | Net |
| --- | ---: | ---: | ---: |
| Shared PIT production | 1 | 1 | 0 |
| Shared PIT and board fixtures | 26 | 13 | 13 |
| Shared manifests | 4 | 4 | 0 |
| MyOB300 board fixture | 1 | 1 | 0 |

Cached focused tests passed 7/7 on each width, including the two manifests,
PIT contract, 8253 exclusion, read-back selection, waveform and MyOB300 board.
The updated read-back fixture failed on both widths when linked against an
isolated copy of the previous PIT implementation.
The S1 scratch executable exercised the same one-line decoder fix with the
unchanged original card. The original volume was unavailable during the S2
production run, so a fresh real-card replay remains pending. The protected
mode shutdown reported by S1 remains a separate issue.

The x64 unit inventory recorded 716 passes and identified the board-level
fixture above. After rebuilding that fixture and updating the test manifest,
both impacted x64 registrations passed. The x86 final inventory passed
717/717 in 131.69 seconds. No test assertion or configured timeout was reduced.

| Log | SHA-256 |
| --- | --- |
| x64 complete inventory before board fixture correction | `a391bbd28de7e403fc2705e6ee7716158da5ac1d2275195f65eb53a52055786d` |
| x64 board and manifest after final correction | `746492b2a803935c46899c53fccfcd1f4933003aaa5fe1793e1ee8c2221daf19` |
| x86 final complete inventory | `107b2319461865d4cca7f01ad3041c2c6712d88876ed15b505a5c7c7e133ba4c` |
| Cached seven-product dual-width publication | `1b6e45966d085be54da447bd5af3f7a70c32efc5fa42f37fb4ac5cc740d9b593` |

All fourteen receiving Release artifacts kept their existing version and
matched their publication log and PE machine type. This verifies deployment,
without claiming a desktop run or external-media integration in this S.

## Published receivers

Shared production source is fixed by commit `530e40076`. The following
hashes match the stable cached publication build outputs.

| Artifact | Architecture | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/nxvm_0_5_0582_x64.exe` | x64 | `ae2f15c41e4a9157980dc47e7ad2468a5dac3d8ca2e920f4a47011345763b529` |
| `assets/nxvm/nxvm_0_5_0582_x86.exe` | x86 | `8a7b86b665a3f4d015d210597cdf2f86a64e84f98a595b0298de96e80ce7b8a8` |
| `assets/my5160/my5160_0_5_0582_x64.exe` | x64 | `92556c982b8e76a3c93116800309a50fc3ba342e44aa6edf5298872154afdeb7` |
| `assets/my5160/my5160_0_5_0582_x86.exe` | x86 | `b921c2847a8a4d1b8f607980d8fb61052b0256d1f229db77f7d37f691c589858` |
| `assets/my5170/my5170_0_5_0582_x64.exe` | x64 | `b2f23d9027b877e5006c6ec6b2420145a138642b89e54f755423a13c4653f1f3` |
| `assets/my5170/my5170_0_5_0582_x86.exe` | x86 | `017787d27744c609f782bf901cbd95aca6af43755c226223ec521193426b1341` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe` | x64 | `750df79f8a73422491bfe0ae90d220efec871005154202b0c923a564849ecdbe` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe` | x86 | `b07e23df35d51ae2093e2715662cbde609ef1b0dd0496b4d1dc2b56117c852b2` |
| `assets/mypc110/mypc110_x64.exe` | x64 | `535954dae1b3c658a90565493028968caabcaeaba90b3dd5453552984788cf24` |
| `assets/mypc110/mypc110_x86.exe` | x86 | `0efbf4cd9b0cc8a58a2b57a64ab59c19fb6e762c35f29935f51d263c898dbd55` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | x64 | `3079bf8dfeb519de7f4e7e7c75bde60cd7097610e9668a33aa1c94e5cb26c520` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | x86 | `dd89929a052791bdccd97534eefdb238c0c5cb4fb42fc2f918221eb622edd553` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | x64 | `da589fa879ddc6c79457231cac1454ab5c013fc7afbce251fa5a67b75227fd3e` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | x86 | `e8af3c3ae7dc329cb4500b7d260f1da2027b420fa3607cd450c4680b1b7224c7` |
