# M4 T27 Shared Corpus Realignment

## Outcome

MyNes now adopts SoftPC revision `a2fbf5488cd0a768cbb5dc92d6e23706bf86b32e`
as the byte-identical source for `src/lib`, `src/common`, `test/lib`, and
`test/common`. MyNes-specific behavior remains outside those roots.

## Delivered Work

- Replaced all four shared roots, including their manifests, source, CMake
  targets, platform leaves and tests. The imported corpus adds SoftPC's native
  audio tests and its Win32 audio vocabulary leaf.
- Adapted Core's APU boundary to the imported Lib stream contract: Core mixes
  in i32, saturates each accepted frame to i16, and uses Lib's query/enqueue/
  clear lifecycle.
- Adapted App's lifecycle bridge to retain MyNes window behavior. A reset
  initiated while stopped is presented to Common as its existing paused path,
  so no Window opens until `start` or `resume`; App still reports the reset
  completion to the cooked monitor.
- Updated product-owned Core and integration tests for the imported audio and
  initial Common `INIT` lifecycle contracts.

## Verification

The four roots have matching file sets and SHA-256 values against the pinned
SoftPC checkout: 97 Lib source files, 23 Common source files, 49 Lib test
files, and 20 Common test files. SoftPC's worktree remained clean.

Both Windows architectures configured and built successfully. The 112
registered CTest cases passed on x64 and x86; x86's two native audio cases were
re-run after its initial parallel build completed. The native Window smoke
proves that a paused initial reset has no Window and `resume` creates it.

The retained artifacts were rebuilt: `mynes_0_1_0011_x64.exe` has SHA-256
`7275964075A5E0EBC58796B699D47230E2F42060664F48C118B707A37106837D`, and
`mynes_0_1_0011_x86.exe` has SHA-256
`EC6D6FD700FFC1056E58FFB190B566B3C043152169914536000EFA560FA0FE93`.
`assets/roms/*` remains ignored and has no tracked entries.

## Delivery

- P1 admission: `ec70865`.
- P2 implementation: `8a2e9de`.
- P3 closure: recorded by this commit.
- No Git remote is configured. The owner-approved remote-aware rule therefore
  requires local commits and makes push unavailable.

## S2 Corrective Closure

The owner found that MyNes displayed its initial prompt while Session still
held `INIT`; the first `rom insert` therefore received the state-policy
rejection. App now withholds that prompt, reads Common's authoritative initial
stopped state after binding its sink, and relays it to Session before input is
accepted. A configured-ROM startup remains driven by its reset completion.
The App command, media and native Window regressions pass on x64 and x86.
P1/P2 admission and proposal repair are `9bf2d31` and `2e9b480`; P3 is
`012bbbc`; this commit closes S2 locally.

## S3 Core Host-Adapter Closure

S3 closes through implementation commit `849fb44`. Core, rather than the
unchanged imported Lib, now owns controller adaptation for KVM's neutral input
events. WASD remains the D-pad; J/Ctrl is B; K/Alt is A; Enter is Start; and
both Shift keys are Select. Each source retains individual binding state, so
releasing one physical modifier side cannot release its concurrently-held mate.
The scan-less virtual-key binding covers RDP and virtual keyboards.

The same corrective work makes a missing native WaveOut endpoint an optional
audio-sink failure rather than a failure to construct the NES. This permits
video and keyboard operation in audio-less RDP sessions. The audible-output
acceptance remains outside this S3 closure and transfers to the admitted S4
shared-audio refresh.

x64 and x86 rebuilt successfully. The Core input contract, imported Win32
keyboard and KVM keyboard-lifetime tests pass; the two rebuilt deliverables
were retained under `assets/binary/`. No shared root changed and no remote is
configured, so `849fb44` is local-only.

## S4 RDP Text-Input Closure

S4 closes through `bdc70ff`. Some RDP and virtual keyboard paths produce a
text record without a virtual-key transition. Core now turns mapped text
records into a single executor-slice controller pulse: carriage return and
linefeed produce Start, and text WASD/J/K produce their respective actions.
The pulse is cleared after the slice, so text input cannot become a held button;
the existing physical and virtual-key source state remains unchanged. The owner
confirmed that Enter works under RDP. This closure does not change Lib/Common.

## S5 Audio-Experience Closure

S5 closes through `658dfed`. The owner accepted the repaired audible result
after comparing the new executable with the retained pre-import x64 executable:
the PCM path now uses a Lib FIFO and stable 1024-frame output batches, while
Core supplies bounded PCM and RDP text Start remains long enough for guest
polling. The comparison executable is retained only as an owner test aid.

The subsequent shared-capability audit found that this behavior-focused repair
is not yet eligible for unchanged SoftPC import: its worker lifecycle, FIFO
wait semantics, failure retention, explicit Base dependency and cancellation
contract require a dedicated S6 hardening pass. S5 therefore closes the owner
experience outcome without claiming shared-corpus identity.

## S6 Shared-Audio Hardening Closure

S6 closes the transfer gap identified by S5. Lib Audio now depends only on
Types and Base, exposes Types-only public vocabulary, owns the producer FIFO
and its worker/native lifecycle, retains a batch until native acceptance, and
uses FIFO capacity for both query and producer waiting. `cancel_wait` wakes one
concurrent waiter; native timeout retries without loss; a native failure is
terminal and visible to producers. Clear/deactivation synchronize reset through
the worker, and destruction joins before it destroys the native sink.

The Lib dependency verifier and its exhaustive self-test now name the admitted
`audio -> base` edge. x64 and x86 each passed all 112 repository tests. The
x86 native audio probe can find no waveOut endpoint over the current RDP
session and correctly treats that as unavailable hardware; deterministic stream
and Win32-platform tests cover the platform-independent and callback contracts.
The rebuilt artifacts have SHA-256 values
`572F644B56F06893A8FF4134286FFF89A75096A5F3E0ED4EA85E4FC496687D06`
(x64) and
`4355C90C8892CD1CF30EA06D6DCAD5DB5DD1C91FDFA9364DC9FD583F3E955C0C`
(x86). SoftPC remained unmodified. Its owner may import `src/lib/audio/` and
the related build, verifier and test changes unchanged, then update that
repository's manifests and run its own suite.

## S7 Corpus-Readiness Closure

SoftPC's follow-up audit correctly found that S6 still used idle and native
timeout polling, and needed stronger cancellation/teardown proof. S7 removes
both timeouts. Base events drive idle FIFO waiting; the Win32 backend waits for
either completion or a dedicated interruption event. Clear and destroy signal
that event before waiting for the worker, so native waits do not outlive their
owner.

The Audio target and verifier explicitly declare `audio -> base`; the native
Audio consumer links Audio rather than relying on a product's link closure.
Deterministic tests now hold native wait while filling the FIFO, then prove
native write failure becomes a producer-visible terminal error. They also prove
destruction interrupts native wait, joins the worker and prevents a later write.
SoftPC can therefore import the Lib Audio corpus unchanged and limit its own
work to converting its LazyBeep requests into PCM in Core/Compat.
