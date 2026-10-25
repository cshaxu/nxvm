# T580 S11 — F65535 panel-table source gate

## Question

Open-Source-PC110 documents a TFT-upgrade table of F65535 extension registers
written by the selected firmware's video initialization. This S asked whether
that table is reached by the already admitted real-firmware path from C000
video ROM back to the `F000:EA86` port-61 wait.

## Method

An uncommitted, removed diagnostic tracer observed only the existing F65535
extension index/data ports (`3D6h`/`3D7h`) while preserving the established
`--c000-port61-return` stop condition. It tracked the complete source-listed
set together: XR06, XR19, XR1A, XR1B, XR4F, XR50--XR54, XR64--XR67, XR6C and
XR6F. It did not write a port, alter firmware bytes, add a timing source or
change a production component.

The hash-validated TFT BIOS and font came from the external BYOB
`profiles-nxvm/ibm-pc110-tft` manifest. The same probe was built and run on
the stable x64 and x86 Ninja/ccache graphs.

## Result

Both hosts reached the established post-C000 return with the same facts:

- `11,015,673` retired instructions;
- one reset and no delivered exception or Core fault;
- `828,865` C000 retirements;
- the established return to `F000:EA84`; and
- zero observed writes for every tracked F65535 XR table register.

Thus the source table is not a receiver for this checkpoint. The result does
not dispute the table or controller model; it only prevents an unsupported
claim that the real firmware has initialized the panel here.

## Disposition

No source or test tracer remains. No Lib, Emulator, X86 Core, X86-NXVM or
MyPC110 production file changed. A later PC110 receiver must identify a
source-defined event beyond this return before modelling panel power or
scanout. Extending a raw instruction limit or injecting the table would not be
valid evidence.
