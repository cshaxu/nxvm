# T422 S1 D4 Slowdown Hold

`M5:T422:S1:D4-SLOWDOWN-HOLD:OK`

Original D3PE evidence defines the 8042 active-low SLOWD output, refresh-triggered auxiliary 8254 channel-2 mode-1 one-shot, and CPU hold while DMA continues. Core owns one route: the KBC output callback, D4 refresh gate, auxiliary PIT and existing run loop. Model-40 selects channel 2; no 5170 route is configured. The regression proves SLOWD selection, refresh trigger, one-shot release, reset, and two non-retiring ticks with zero CPU retirement. No DCLK calibration, physical 250 ns scalar, ROM/media import, or L3 claim is made.