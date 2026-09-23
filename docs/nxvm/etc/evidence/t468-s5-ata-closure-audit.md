# T468 S5 ATA Closure Audit

## ATA disposition

The frozen ATA-3 Checklist 1 and code-gap Checklist 2 contain fifteen rows.
S3 closes task-file, reset and IRQ-mask behavior; S4 closes PIO write media
admission. S5 rechecked every row against those sole owners: `hdc.c` owns
task-file, phase and logical-sector selection; the existing media provider
owns media transfer; PIC remains the sole delivered-IRQ route. No second ATA
path, cache, task-file mirror or public ABI was added.

The static ATA ownership gate passes with the shared sector resolver and no
retired CHS/LBA helper names. Focused HDC, VM ATA, Compaq and Model-40 HDC
smokes pass.

## Gate-discovered repair

The serial gate exposed an unrelated but concrete VADP closure defect. CGA
registered `3DBh/3DCh` first, then the Compaq CECG personality attempted to
register the same physical ports. VADP now keeps the original port entries
and their one handler selects the already-owned CECG latch state only when
that personality is active. The non-conflicting monochrome pair remains
registered by the same handler. This removes duplicate registration; it does
not add a video path or a second state owner.

The same gate then exposed stale VADP test assumptions and a firmware defect:
bare VADP reset intentionally leaves output disabled, so direct tests enable
it explicitly; CGA CRTC reads retain T467's write-only grammar; and the BIOS
text-mode service now writes mode `0Dh`, rather than disabled `05h`. The
firmware and guest continue to reach the one VADP state and copied snapshot
route.

## Verification and artifact

- `cmake --build --preset current-gates-gcc`: static, documentation and
  specialized gates pass.
- Its serial `current-gate` run passes **295/295** in 159.84 seconds.
- `cmake --build --preset current-gcc` produces
  `build/output/nxvm_0_5_0468.exe`.
- SHA-256: `99100D6B99ABD98FF584F6B4179B6BC8B96519B53F234FBE4CC45DDBAC860BF5`.
- `objdump -h` finds no `.debug` section. The runtime debugger remains a
  product path; only compiler debug information is absent.

The changed source/test paths are 61 additions and 71 removals (-10). The
only retained exceptions are the exact ATA L2 transfers already declared by
Checklist 1/2; this S adds none.
