# M5 T421: DeskPro D4 Speaker Line

T421 implements the original-source-defined DeskPro port-`61h` speaker-line
logic without creating a Model-40-only device. The Core owns port-B state,
PIT-channel-2 gate and output sampling, reset behavior and a copied
observation. Model-40 selects its existing D4 port-B composition; IBM 5170
selects its existing planar composition. Both use the same Core route.

This closes only the logical digital line. Audio rendering, acoustic and
physical-frequency behavior, host-time scheduling and broader PPI semantics
remain transferred in the hardware debt ledger. T421 is not a physical timing
or Model-L3 acceptance.