# M5 T422: DeskPro D4 Slowdown Hold

T422 S1 implements the original-source logical D3PE SLOWD* path with one Core owner. The 8042 output-port bit 3 enables a refresh-triggered auxiliary-PIT channel-2 mode-1 one-shot. While its output is low, the existing Core run loop publishes non-retiring time and continues device scheduling; the CPU does not retire. Model-40 alone configures the route; IBM 5170 has no binding.

The focused regression proves low-active selection, refresh trigger, one-shot release, reset isolation and two elapsed non-retiring ticks with zero CPU retirement. The full current gate and documentation governance pass. This is not a DCLK calibration, physical waveform, media/ROM input, or Model-L3 acceptance.