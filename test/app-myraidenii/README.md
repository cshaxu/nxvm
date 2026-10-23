# MyRaidenII Tests

M1 T1 provides a ROM-free unit for the inventory verification tool, and M1 T3
adds a Board-Core route unit with synthetic program lanes. Neither uses the
game ROM. The product is registered in the repository build graph; its
standalone CMake entry remains available for focused development:

```powershell
cmake -S test/app-myraidenii -B build/.ninja/gcc-x64-release -G Ninja
ctest --test-dir build/.ninja/gcc-x64-release --output-on-failure --no-tests=error -L myraidenii
```

Runtime units will mirror real source owners. Shared external harnesses belong to setup,
owner-ROM cases to integration, opt-in probes to diagnostic. Introduce each
directory with actual tests. Use [the source map](../../docs/myraidenii/design/CODING.md),
ROM-free owned fixtures and the existing common registration helper for C tests;
never borrow private App fixtures or count an unexecuted diagnostic as a pass.

`integration/selected_set_startup_probe.c` is an explicitly invoked owner-ROM
diagnostic, not a CTest case. It accepts two program lanes and seven prepared
graphics-role files of the existing fixed sizes; the external staging wrapper
owns archive validation and cleanup. Its output contains only stop/checkpoint
metadata and neutral frame statistics, never ROM bytes, pixels, register values
or a raw trace.
