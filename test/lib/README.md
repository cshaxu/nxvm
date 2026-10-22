# Shared Lib tests

Transfer `src/lib`, `test/lib` and the shared CMake tools directly in `test/`.
This suite needs no other source or test package.
Transfer the shared CMake tools directly in `test/` with any test package.
This suite requires only `src/lib`, `test/lib`, those tools, a C11 compiler and system
libraries. No product target, firmware, INI or media is required.

```text
cmake -S test/lib -B build/lib-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build/lib-tests
ctest --test-dir build/lib-tests --output-on-failure -LE "desktop|native_audio"
```

The suite owns test registration, fixtures, failure injection and its manifest.
Production verifiers remain in src/lib. Platform probes are selected explicitly
by CMake; Linux fakes test algorithms, not native Linux runtime availability.
Assertions remain enabled in Release. Scratch files stay in the build directory.
The shared file cleanup helper belongs to this suite's storage probes.

The four negative source fixtures are flat files in fixtures/. Configure copies
each into its required source-owner layout under the build directory. Each
negative test must still match its intended diagnostic, not merely fail.
Test targets use the complete relative test path; equal Win32/Linux leaf names
therefore remain distinct targets and working directories.

## Platform coverage

Matching capability tests use matching filenames. Owner-root suites are reused,
not copied into platform directories; adapter tests use controlled native calls.
Coverage alignment does not mean duplicating Windows-only behavior
for an unsupported backend, or making both platforms' source bodies identical.

| Owner | Common coverage | Platform coverage and intentional difference |
| --- | --- | --- |
| Types | Scalar/layout/atomic contracts and platform-header compilation gate | Header-only vocabulary, no backend runtime to duplicate. |
| Base | Clock arithmetic, text, task rejection; process-directory test applies the same capacity/failure checks to both actual path leaves | Both clock_failure and sync probes inject native failures; both keyboard_decode probes cover mapped identity, make/break and repetition. Windows also maps physical sides/keypad and host layout; Linux terminal identities have no physical scan or scalar-to-key mapping. |
| Storage | Binary writer, positions, UTF-8 paths, lock/mode and close contracts | Both file probes test open mode, invalid encoding and failed-acquisition cleanup. Linux additionally checks flock via file_lock; Windows sharing is selected in file and exercised by common storage tests. |
| PCM | Shared FIFO, progress, cancellation and lifetime with a fake backend | Both stream probes execute their actual backend: Windows exercises native queue/initialization/failure; Linux proves public create rejects with UNSUPPORTED/null and checks stub output counts. |
| MIDI | Short/SysEx validation, capacity, cancellation with a fake backend | Both output probes execute their actual backend: Windows has native ownership/failure cases; Linux rejects acquisition and cannot send messages. |
| Console | Logical events, output and blocking gates | No platform directories; native I/O belongs to the broker. |
| Console broker | Binding and failed create | Both console probes exercise native boundary contracts. Windows owns screen/input/reader behavior; Linux proves acquisition/output/activation rejection and cancellation output. |
| KVM Base | Mailbox, key lifetime, matching, input delivery and text contracts | No platform implementation directories or artificial platform tests. |
| KVM Console | Frame/input contracts and failure output | Windows has an actual worker with retirement/keyboard/shutdown tests; Linux component tests public acquisition failure, null result and safe repeated create/destroy. |
| KVM Window | Geometry, frame/motion/damage and failure output | Windows has native capture/message/keyboard tests; Linux component tests public acquisition rejection and safe repeated create/destroy. |

Unit directories follow src/lib ownership: types, base, console,
console-broker, storage, audio/{pcm,midi,fm}, kvm-base, kvm-console and
kvm-window. Tests use only their owner and its inward production dependencies;
assertions about independent inner contracts stay with the inner owner.
Using several components does not turn a unit into an integration test.

The `desktop` label marks real native Window/Console tests, including tests
that hide a window only after creating it. Run these explicitly with `-L desktop
-j 1` when the desktop is reserved for testing. Mocked input/rendering tests
remain in the background suite. Real-host tests live in integration/ and are
not labelled unit. Native MIDI requires a usable host endpoint and carries
native_audio; run it explicitly with -L native_audio. Unit tests may use system
threads/files but not a logged-in desktop, keyboard layout or audio device.
An unfiltered CTest run includes all groups; missing native capability is not
evidence of playback, even when an availability probe reports it successfully.

Coverage: Types/atomic/clock, overlapping moves and text vocabulary; logical Console event/output gates;
Console broker, display and reader failure; Base sync lifetime and Linux waits; Storage
binary truncate/append writer, mode lock matrix, medium lease replacement/failure preservation and consumed-close
failure; KVM physical/text input,
matcher/frozen admission, frame damage, geometry/motion, control FIFO,
capture, component frame publication, terminal mailbox admission, modal wake,
failures and retirement; Audio queue, cancellation, partial delivery and backend lifetime. Native GUI smoke does not prove
every terminal host or interactive product scenario.
