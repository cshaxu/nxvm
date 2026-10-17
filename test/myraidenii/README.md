# MyRaidenII Tests

M0 is documentation-only; there is no registered test or test-only product.
M1 units mirror real source owners. Shared external harnesses belong to setup,
owner-ROM cases to integration, opt-in probes to diagnostic. Introduce each
directory with actual tests. Use [the source map](../../docs/myraidenii/design/CODING.md),
ROM-free owned fixtures and the existing common test registration helper;
never borrow private App fixtures or count an unexecuted diagnostic as a pass.
