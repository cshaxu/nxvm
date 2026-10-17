# MyRaidenII Tests

M1 T1 provides a ROM-free unit for the actual inventory verification tool.
It checks synthetic accepted data and isolated failures, never the game ROM.
Run the product-owned registration without changing a Shared build graph:

```powershell
cmake -S test/myraidenii -B build/myraidenii/tool-tests -G Ninja
ctest --test-dir build/myraidenii/tool-tests --output-on-failure --no-tests=error
```

Runtime units will mirror real source owners. Shared external harnesses belong to setup,
owner-ROM cases to integration, opt-in probes to diagnostic. Introduce each
directory with actual tests. Use [the source map](../../docs/myraidenii/design/CODING.md),
ROM-free owned fixtures and the existing common registration helper for C tests;
never borrow private App fixtures or count an unexecuted diagnostic as a pass.
