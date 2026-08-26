# M5 T478 S4 Closure Audit

## Scope and conclusion

T478 closes the ordered `default-at` migration. IBM 5170, DeskPro Model 40,
and Default-AT each enter Core through one immutable resolved-profile snapshot
and one copied Core-input plan. The session owns only the dynamic resources
that it constructs; no direct Default-AT Core materialization or override path
remains.

## Release evidence

| Item | Record |
| --- | --- |
| Source revision | `1d388abd3592e3ce2e653fabf686b2297e60cb98` before the closure/documentation commit |
| Current target | `vm-0-5-0478` |
| Artifact | `build/output/nxvm_0_5_0478.exe` |
| Banner | `Neko's x86 Virtual Machine [0.5.0478]` |
| SHA-256 | `6E34BBDFF74C6BAED6EFBF91FA00A432B4736427018BE9834A4095A0C300E61C` |
| Debug information | Release `objdump` section scan found no `.debug` or `debug_` section. |

## Verification

- The frozen Default-AT/IBM 5170/DeskPro profile smoke corpus passed, including
  the root resolver, Model-339 composition, Default-PC/AT and PC/AT composition
  owners.
- `cmake --build --preset current-gates-gcc` passed all 78 configured steps for
  this active S4 delivery. The intentional T345 negative self-test reports its
  expected diagnostic before reporting success.
- The closure source sweep confirms one resolver-to-copied-Core-input route per
  AT profile; S1--S3 evidence records the field ledger, child resolver, deleted
  session path, and retained dynamic owners.
