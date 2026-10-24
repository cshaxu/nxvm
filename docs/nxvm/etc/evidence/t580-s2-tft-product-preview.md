# M5 T580 S2 — TFT Product-preview evidence

S2 connects the already admitted external TFT firmware/font views to one
ordinary `x86-nxvm/product` composition.  It adds only App-local
`main.c`, a machine binding, `assets/mypc110/mypc110.ini`, and a focused
composition smoke test.

The build continues to validate the external `ibm-pc110-tft` manifest and the
two recorded hashes before generating the ignored firmware byte object.  The
test parses the App-owned INI grammar, constructs the existing 486SX Machine
through `x86_nxvm_product_compose_machine`, confirms its copied product facts
and driver description, then uses the existing ordered destroy callback.

The Product preview is an `EXCLUDE_FROM_ALL` build target.  It exists to compile
the real App entry against the existing X86-NXVM Product route; it is neither
deployed nor published.  No raw firmware, original-DSTN asset, guest disk,
runtime model selection or guest-boot conclusion is introduced.

Both x64 and x86 built the preview and passed all seven MyPC110 unit rows:
`config-route`, `firmware-binding`, `display-route`, `font-rom`,
`machine-construction`, `lpt`, and `product-composition`.
