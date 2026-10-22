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
depth are local L1 approximations. DT1, key scaling, LFO and noise are absent.

## Evidence and limits

The existing Sound unit executes synthetic Z80 programs and checks stereo and
mute routing, all eight algorithms' carrier selection, first-operator modulation
for algorithms 0--6, key-off, octave, total level, all nonzero DT2 settings and
sustain-level ordering. Both x64 and x86 focused runs pass. These observations
do not prove exact waveform, timbre, envelope timing, modulation scaling or
host audio playback.

A bounded selected-set probe on each architecture executed 20,000,000 V30
instructions with coin/start input, no reported board or COP provider error,
739,013 nonzero PCM frames generated and the same retained PCM digest
`097355bc73288e0f`. This is the version with the sustain target fix; it is
not a recording, playback qualification or proof of game completion. The
probe's bounded staging is external and removed by its owner.

The current candidate passed all 698 repository-only unit tests per
architecture (x64 122.81 s; x86 122.83 s). The
MyRaidenII release artifacts, playable audio quality, suspended S79 pause FIFO,
and T7's full gameplay/desktop exits remain unqualified.
