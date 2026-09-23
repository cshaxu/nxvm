# T374 S14: CGA/EGA CRTC Index Gate

VADP now accepts CRTC index `13h` only after EGA controllers are configured.
The CGA-only Model-339 route retains only R10--R15; selecting `13h` makes its
data write inert and data read zero. EGA retains `13h` as its offset register.

The shared gate is `core_machine_vadp_supported_crtc_index()`: it guards CRTC
read, write and paired-word consumers, preventing a second EGA-only escape
route. The sweep covered every caller of that predicate, CRTC mask/word helper,
EGA configuration and Model-339/EGA tests.

Focused rebuild and replay passed:

```text
M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK
M5:T314:S2:EGA-CRTC-BOUNDARY:OK
M5:T228:S1:CGA:PORT:OK
```

R0--R9 geometry/timing, light pen and Model-339 L3 remain open.
