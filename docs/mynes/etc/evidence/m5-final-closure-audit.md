# M5 Final Closure Audit

M5's fixed acceptance matrix is complete on x64 and x86. Each ignored local
input ran 1,000 driver slices in Window graphics and Core-generated text mode,
published 26--28 frames and did not raise a Core trap.

| Role | Mapper | Product evidence |
| --- | --- | --- |
| Dr. Mario | MMC1 | Both presenters, x64/x86 six-input probe. |
| Super Mario Bros. | NROM | Both presenters, x64/x86 six-input probe. |
| Super Mario Bros. 2 | MMC3 | Both presenters, x64/x86 six-input probe. |
| Jackal | UxROM | Both presenters, x64/x86 six-input probe. |
| Super Mario Bros. 3 | MMC3 | Both presenters, x64/x86 six-input probe. |
| TMNT III | MMC3 | Both presenters, x64/x86 six-input probe. |

Mapper 1/2/3/4, persistence, media-failure, lifecycle, App media-command and
controller-ROM regressions passed 9/9 on each architecture. The checked
release manifest verifies the versioned executable pair and sole editable INI.
ROM paths/bytes are not recorded. MMC6, mapper 118/119, multicarts, PAL/Dendy,
expansion audio, additional controllers, save states, replay, rewind and
networking remain explicitly excluded.
