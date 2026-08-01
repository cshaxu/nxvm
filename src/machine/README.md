# Machine

`machine/core` is the sole shared machine-core directory. Its public API and
implementation files are peers; private headers use the `_impl.h` suffix.
`machine/vm` and `machine/vdm` add only product-specific machine behavior.
