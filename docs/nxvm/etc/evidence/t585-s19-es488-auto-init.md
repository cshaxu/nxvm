# M5 T585 S19 — ES488 continuous playback

The PC110 App now implements the Sound Blaster-compatible 8-bit auto-init
output commands on the existing ES488 DMA, IRQ, guest-clock and PCM path.
Command 48h sets the little-endian block length minus one; 1Ch starts automatic
playback; each completed block raises the existing IRQ and reloads the block
count; DAh finishes the current block and stops. D8h reports the speaker state.
Reset and the existing pause/resume path remain the owners of their behavior.

This is an L2 behavioral model, independently implemented with reference to
86Box `src/sound/snd_sb_dsp.c` at commit
`4fef696a4eead1d55a28d6ac0e5bd2864e5454da`. It does not establish physical
ES488 timing, complete ESS command coverage, capture, or FM fidelity. No
reference source or external media is copied into the repository.

The existing machine fixture programs the real DMA controller for automatic
reload, verifies three two-byte blocks, checks and acknowledges each block IRQ,
and checks graceful exit at the final boundary. The existing composition test
programs DMA, DSP and FM using guest CPU instructions and checks mixed PCM when
the native endpoint is available. On this run the isolated desktop reported
`HOST-UNAVAILABLE`; native submission and audible output were therefore not
verified, rather than counted as successful output.

Both architectures were rebuilt and the deployed PE architecture checks passed.
The PC110 suite passed 20/20 on each architecture, including the real firmware
boot and resume integrations. Documentation governance and `git diff --check`
passed. The two deployed assets have SHA-256:

* x64: `422287361a1bf96e50fcd1d090155786559be31d4364107d93d314ee9db293cb`
* x86: `189ba47fcbc76bcd1e9f12c19ee73fc1da67bc0e873c316c6348193864d8f5c9`

No Shared component, owner configuration, firmware, or media was changed.
