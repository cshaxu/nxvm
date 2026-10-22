# T7 S81: MyRaidenII YM model, work in progress

The local sound owner now uses four individually keyed sine operators per YM
channel, one of eight connection graphs, stereo routing and the existing PCM
queue. This replaces the earlier diagnostic square waveform; it does not
establish YM2151 fidelity or complete the sound implementation.

## Independent behavioral reference

The local 86Box checkout at `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`,
`src/sound/ymfm/ymfm_opm.h`, `ymfm_opm.cpp`, `ymfm_fm.h` and `ymfm_fm.ipp`, was inspected
for register fields, operator connections, nominal DT2 offsets and sustain
level encoding. Those finite facts are L2; no source or table is incorporated.
DT2's nominal 0/600/781/950-cent settings are evaluated as continuous
frequency ratios, not the reference's quantized phase computation. The special
sustain setting 15 corresponds to 31 attenuation steps; each step is modeled
as 3 dB. Equal-tempered pitch, continuous phase, envelope rates and modulation
depth are local L1 approximations. DT1 is absent. A later candidate applies
key-dependent envelope-rate scaling with zero attack/decay/sustain remaining
zero and a bounded effective rate. The rate-selection behavior is based on
the inspected OPM reference; the resulting continuous envelope timings remain
an L1 approximation. A synthetic attack test checks relative key scaling and
zero-rate behavior on both widths. The current candidate's 13 MyRaidenII
units pass on each width. Each selected-set probe reaches 20 million
instructions with 736,536 nonzero generated PCM frames and matching PCM digest
`ea90f8cea7f30039`; this is not a fidelity or desktop-playback result.
The full repository unit selection passed 698/698 on x64 and 698/698 on x86,
run sequentially after the probes.

An absolute pitch comparison against the same pinned 86Box reference's
`ymfm_fm.ipp` key-code conversion and `ymfm_opm.cpp` phase-step use found that
key code `0x4c`, zero fraction and multiplier one at the nominal board clock
is approximately 466.2 Hz, not the local model's former 440 Hz. This is an
L2 comparison to an emulator, not a claim that every phase step is exact. The
local equal-tempered approximation now uses the corresponding A-sharp base;
no reference table is copied. The existing pitch test measures the mean period
over multiple PCM zero crossings. On x64 it failed with the former base and
passed with the corrected base; all 13 MyRaidenII units and the executable build
pass on each width. Sequential selected-set frame-sequence runs after the pitch
change each reached 20 million instructions without a Board/COP failure; both
widths produced 736,511 nonzero PCM frames and matching PCM digest
`b186a68211d94251`, with matching frame digest `ac25ef796f90ea94`. The PCM
digest differs from the pre-change model, as expected; this is not a listening
test or desktop playback qualification.

The same reference's `ymfm_fm.ipp` `engine_timer_expired` and
`engine_mode_write`, together with `ymfm_opm.h`, describe an independent L2
timer contract: an expiration sets its status flag only when that timer's
flag enable is set; a latched flag remains until the corresponding reset.
The local receiver now follows this contract for both timers and uses the
latched flags for the IRQ request. A synthetic Z80 program checks expirations
with flag generation disabled, subsequent enabled expirations and persistence
after stopping; the existing interrupt-delivery test remains in use. The
product's 13 units and executable build passed on each width for this change.
Sequential x64/x86 selected-set frame-sequence probes each reached 20 million
instructions with no Board/COP failure, 736,536 nonzero generated PCM frames,
and matching PCM digest `ea90f8cea7f30039` and frame digest `ac25ef796f90ea94`.
The probe uses the external owner archive, and its staging is removed by the
wrapper. These observations do not establish desktop audio or fidelity.

The same reference's `fm_operator::start_attack` treats an effective attack
rate of 62 or more as immediately reaching the maximum envelope on key-on.
The local model now applies this L2 threshold while retaining its L1 gradual
envelope approximation at lower rates. The existing pitch test additionally
checks the early output level; the key-off test observes both initial sound and
the eventual release after a longer window, rather than assuming the old
slower attack's lower level. The MyRaidenII units and executable builds pass
on both widths. Selected-set and desktop playback have not yet been rechecked
for this attack change.

The local Machine audio clear path previously left Board PCM behind while
discarding staging and clearing the host stream. Since heartbeat shutdown is
called at the executor boundary, the retained Board frames could be submitted
on resume. The Machine bridge now drains and discards those frames after a
successful host clear, without another queue or Shared API change. The existing
Machine audio fixture checks that a failed host clear leaves Board data intact,
and that successful clear removes more than one submission's worth before a
later drain. The no-stream path and staging-failure checks remain in place.
Both-width MyRaidenII units and builds are checked separately from desktop
audio or actual pause/resume qualification.

An in-progress LFO candidate keeps the two `0x19` depth writes separately and
uses channel PM/AM sensitivity and per-operator AM enable. The selected
rate and four waveform selections follow the inspected register vocabulary;
oscillator phase, continuous waveforms, modulation scaling and the held
noise waveform are L1 approximations. The phase-modulation sensitivity's
nominal cents values are finite facts recorded by the same reference, not a
copied implementation. Tests exercise disabled routing, AM, PM and waveform
selection. Both product unit selections pass 13/13 and both selected-set
probes complete 20 million instructions. These checks do not establish
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
