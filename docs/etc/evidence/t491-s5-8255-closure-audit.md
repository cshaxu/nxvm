# T491 IBM 5160 8255 PPI, Keyboard And NMI Closure Audit

`M5:T491:S5:8255-CLOSURE:IMPLEMENTED-PENDING-GOVERNANCE`

## Coverage and disposition

The frozen unit is the 19 rows in T491 S2 List 1. F1--F4 and F6--F9,
F11--F14 and T2--T3 remain in the sole Core XT PPI/NMI route. F10 is now
complete: the PPI owns the Port-B latch and publishes only PB0/PB1; the
existing Core board owner consumes those two lines, owns PIT2/speaker state
and publishes the copied speaker observation. F15 remains an explicit
external keyboard-device input boundary. F5 remains L0 for unwired selected
XT Mode-1/2 behavior; T4 remains L1 because the IBM source gives no numerical
keyboard-clock interval; T1 remains L4/out of scope. No List-2 gap remains.

## Simplicity and sweep

The sweep inspected all tracked PPI Port-B write/reset sites and every Core
PIT2-gate/speaker ingress. The selected XT, planar-parity and DeskPro sources
all select one neutral Core speaker consumer; no Port-B mirror, second PIT,
port provider, scheduler or runtime profile setter exists. From the parent of
S1 through S4, the tracked source/test surface is 101 added and 18 removed
lines (net +83; `git diff --numstat 44357022^..36539a65 -- src tests`). The
increase is the two-line typed publication boundary, its sole stored board
inputs and focused proof; no obsolete production path was retained.

## Verification and artifact

Focused PPI/PIT CTest passes 3/3. A fresh independently logged Debug current
gate passes 300/300. Documentation governance passes. The configured stripped
Release target is `vm-0-5-0491`; its emitted developer artifact is
`build/output/nxvm_0_5_0491.exe`, SHA-256
`1C219347EEF080574486E5D62D18462E8764F77900D34755897A9A87BA032E7E`.
The CMake target supplies `PRODUCT_BUILD_VERSION="0.5.0491"`; the runtime
debugger remains a product capability and compiler debug information is not
present in this Release route.

The next queued unit is the independent IBM 5160 8272A FDC/media task; this
closure transfers no untracked PPI work into it.
