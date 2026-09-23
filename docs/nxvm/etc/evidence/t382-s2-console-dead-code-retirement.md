# T382 S2: Console Dead-Code Retirement

`M5:T382:S2:CONSOLE-RETIREMENT:OK`

## Result

The frozen Console keeps one reachable route for `SESSION`, `HELP`, `EXIT`,
`INFO`, `DEBUG`, `RECORD`, `FLOPPY`, `START`, `RESET`, `STOP`, and `RESUME`.
The stale block after the completed `FLOPPY` command and the private `SET` and
`TEST` helpers are removed. The Console provider no longer exposes old boot,
memory, FDD-create, or HDD mutation callbacks; the general session APIs remain
because profile construction still owns them.

The legacy Console test support now recognizes only the retained `FLOPPY`
media command instead of retired `DEVICE` and `SET BOOT` syntax.

## Sweep

The production and focused-test search covered `src/vm/product`,
`src/vm/composition/session`, `tests/product`, and `tests/products` for the
retired command names and the removed provider hooks. The only remaining
`DEVICE` matches describe the generic VM device thread or display error text;
they are not Console commands. Remaining session boot/HDD/FDD construction
APIs are profile/session construction owners, not Console adapter callbacks.

## Verification

- Fresh GCC/Ninja configuration in `build/t382-s2-gcc` compiled the complete
  current graph.
- Focused current Console input-failure, lifecycle, and Console model smokes
  passed (3/3); the catalog smoke rebuilt with the retained Console product.
- The fresh `run-current-smokes` and `verify-current-specialized-gates`
  invocation completed successfully before the executor's final owned-process
  cleanup check.
- Documentation governance and `git diff --check` pass before P1.

## Artifact

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0382.exe` | `6E0B396BEA66EEFB8B4A1BE60B66C6115F50553E9C14B8623DB8BFC2E2B755CA` |
