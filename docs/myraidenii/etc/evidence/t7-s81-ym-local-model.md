# T7 S81: MyRaidenII YM model, work in progress

The local sound owner now uses four individually keyed sine operators per YM
channel, one of eight connection graphs, stereo routing and the existing PCM
queue. This replaces the earlier diagnostic square waveform; it does not
establish YM2151 fidelity or complete the sound implementation.

## Independent behavioral reference

The local 86Box checkout at `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`,
`src/sound/ymfm/ymfm_opm.h`, `ymfm_opm.cpp` and `ymfm_fm.ipp`, was inspected
for register fields, operator connections, nominal DT2 offsets and sustain
level encoding. Those finite facts are L2; no source or table is incorporated.
DT2's nominal 0/600/781/950-cent settings are evaluated as continuous
frequency ratios, not the reference's quantized phase computation. The special
sustain setting 15 corresponds to 31 attenuation steps; each step is modeled
as 3 dB. Equal-tempered pitch, continuous phase, envelope rates and modulation
depth are local L1 approximations. DT1 and key scaling are absent.

An in-progress LFO candidate keeps the two `0x19` depth writes separately and
uses channel PM/AM sensitivity and per-operator AM enable. The selected
rate and four waveform selections follow the inspected register vocabulary;
oscillator phase, continuous waveforms, modulation scaling and the held
noise waveform are L1 approximations. The phase-modulation sensitivity's
nominal cents values are finite facts recorded by the same reference, not a
copied implementation. Tests exercise disabled routing, AM, PM and waveform
selection. Both product unit selections pass 13/13 and both selected-set
probes complete 20 million instructions. Full repository units remain to be
run on this candidate before S closure. These checks do not establish
quantized OPM behavior or audio fidelity.

The subsequent local noise candidate uses the reference's register-selected
noise output on channel 7 operator 4 and a nominal rate derived from that
clocking model, but a different Galois generator and PCM-rate sampling. Its
sequence and spectrum are L1 approximations, not an exact OPM noise claim.
The selected board's YM clock of 28,636,363/8 Hz is also visible in the local
`mame-raiden2-l2/raiden2.cpp` reference at latest-path commit
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff` (SHA-256
`fdde57f6e9024657f2a9830864523305139562229750b87c967e597324cdb26a`),
an L2 board observation rather than a source import.

## Evidence and limits

The existing Sound unit executes synthetic Z80 programs and checks stereo and
mute routing, all eight algorithms' carrier selection, first-operator modulation
for algorithms 0--6, key-off, octave, total level, all nonzero DT2 settings and
sustain-level ordering. The noise candidate additionally tests that enabling
noise changes channel 7 but not channel 6. Both x64 and x86 focused runs pass. These observations
do not prove exact waveform, timbre, envelope timing, modulation scaling or
host audio playback.

A bounded selected-set probe on each architecture executed 20,000,000 V30
instructions with coin/start input, no reported board or COP provider error,
739,013 nonzero PCM frames generated and the same retained PCM digest
`097355bc73288e0f`. This is the version with the sustain target fix; it is
not a recording, playback qualification or proof of game completion. The
probe's bounded staging is external and removed by its owner.

The current candidate passed all 698 repository-only unit tests per
architecture (x64 122.81 s; x86 122.83 s) before the noise addition. With
noise, full tests on both architectures were started concurrently with two
real-set probes: each full run had timeouts in `lib.test-boundaries-selftest`
and `emulator.verifier-negative`. After all probes exited, both tests passed
in isolation on each architecture (x64 6.06/11.70 s; x86 7.70/18.79 s).
This is consistent with contention but does not prove the timeout cause.
Architecture-by-architecture full reruns with six workers then passed all
698 tests: x64 101.65 s and x86 93.60 s. Both noise-candidate selected-set
probes also completed their 20 million instruction budgets. The
MyRaidenII release artifacts, playable audio quality, suspended S79 pause FIFO,
and T7's full gameplay/desktop exits remain unqualified.
