# T366 S6: Model 339 CGA Topology

The Model 339 profile now selects CGA-only VADP topology. EGA port registration
and configuration are optional; the generic default profile retains them. The
512 KB configuration receives a VADP-owned `B8000h` CGA VRAM provider, so CGA
capture no longer relies on ordinary RAM above installed planar memory.

`vm-ibm-5170-model-339-cga-topology-smoke` proves Model 339 CGA CRTC/mode/
status ports and text capture, absent EGA ports/configuration, and retained
default EGA ports/configuration. It emits `M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK`.

This selects topology only. IBM ROM, exhaustive CGA timing, other adapters,
FDC timing, MFM/ST-506 and bus/cycle timing remain later T366 transfers.
