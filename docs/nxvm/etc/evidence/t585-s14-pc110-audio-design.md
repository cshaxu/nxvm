# T585 S14 — PC110 onboard audio design audit

The selected board has two distinct sources: ESS ES488F digital audio and a
discrete Yamaha YM3812 with YM3014B DAC. Open-Source-PC110 checkout
`02ccf2e8e19b298a2cf04c5d430278eb087cc368`, `Discovery/ES488/readme.md` and
`Discovery/Live-Dump/README.md` report an observed configured Sound Blaster
interface at 220h: reset 226h produces AAh at 22Ah, command E1h returns 2.01,
and command E7h identifies the ESS family. They identify a separate OPL2 at
388h and a DMA/IRQ path. These are reference observations, not a PC110
physical test performed by this repository; clock and unobserved configuration
behavior require explicit L1/L2 choices. The hardware may require software
configuration, so a fixed default must be described as a model, not a claim
about every power-on board state.

Current PC110 composition contains no ES488 or YM3812. X86-NXVM's Board has
PIT/speaker logic and a DMA bus with channel provider/request bindings, but
speaker observation is not a PCM output and there is no X86-NXVM host PCM
stream. Lib already has a bounded signed-16-bit PCM stream with explicit
accepted-frame and cancellation contracts. MyNES and MyRaidenII use that
existing stream, but their product-private sound implementations are not a
PC110 device implementation and should not be coupled into PC110.

The proposed sequence retains one Core guest-time axis, one Board DMA/IRQ
owner, one Emulator Machine executor and one Lib PCM output. Start with the
real guest-visible DSP reset/version/identity protocol and tests; do not call
that audible audio. Then implement guest-driven 8-bit PCM transfer with a
documented nominal timing model, channel/IRQ configuration and explicit
completion/acknowledgement; connect its samples to the existing Lib stream at
the machine boundary with pause/stop/teardown ownership and accepted-prefix
handling. The YM3812 needs its own register/timer behavior and an actual FM
sample generator; timer/status recognition alone must not be sold as music.
Split these into sequential numeric S tasks by demonstrated behavior rather
than introducing a general sound framework or a second execution loop.

DMA channel binding currently belongs to Board, while the PC110 profile
factory runs before the Board's topology DMA initialization. Audio samples
must reach host PCM from the X86-NXVM machine runner, which currently has no
generic PCM producer hook. These two concrete integration seams require
small designs and real receiver tests when those batches are admitted;
neither requires changing Lib or Emulator merely to add a PC110 device.
User-owned media, CMOS and ROM remain untouched. Mixer nuances, actual
physical DAC output and end-user audible behavior require their own evidence.

This is a static source/reference audit, not an implementation or audio
playback test. S11's final source and artifact verification remains the
applicable baseline; S13 withdrew its unneeded, uncommitted port-window draft.
