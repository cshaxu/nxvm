# T580 S71 — Post-Setup Power And Boot Continuation

## Scope and initial evidence

The unchanged selected TFT BIOS, owner-managed CMOS seed and existing
Easy-Setup comparison oracle are used through the real MyPC110 receiver.
S70 had reached a second wait at `00039333h` after Setup and the `102`
correction. The old diagnostic permitted only one explicit F12 event.

A first x64 diagnostic replay observed the existing suspend-refresh signal,
issued a second explicit F12 make/break through the normal driver, observed
processor reset, verified the retained image and a second genuine RSM, and
reached the BIOS INT 19h instruction. The original process result handle was
lost, so that log is observation evidence only and is not counted as a passing
run. No product-side automatic button, firmware hook or CMOS rewrite is used.

The diagnostic now updates its exception-count baseline on every observed
post-Setup reset before comparing the final count. Earlier resets were skipped
by the diagnostic branch, leaving a stale baseline. A final dual-width replay
records independent result files and each real input/restore checkpoint; it is
in progress. The two widths use independent machine instances and scratch
outputs, while firmware and oracle are read-only; they run concurrently on
the 24-logical-processor host. The x64 work has a 1,800-second wall bound, x86 3,600 seconds,
with 300-second no-progress and 64 MiB output bounds. These are containment,
not a success condition.

The final x64 replay has now exited zero. It observes the saved Setup reset,
the actual second suspend at `00039333h`, a second F12 make/break through the
existing driver, a second verified retained-image RSM and the real INT 19h
handoff at `F000:E6F2h`. The final exception counter equals the post-reset
baseline (8) and the diagnostic reports no first fault. Its output SHA-256 is
`D82C212F2333170B35D9EAF62C5708343405D74FDCCE20373070371BE4BD10A1`.
This is an x64 firmware handoff, not a claim of a booted operating system;
the x86 replay remains in progress.

The earlier Setup frame is a 640x480 RGB image. Its dominant guest pixel
colors were black `000000` (137,810 pixels), `BE9E96` (113,923), white
`FFFFFF` (50,030) and `710820` (4,579). The frame therefore includes a
large pink/brown region; this measurement does not verify the actual native
Window rendering, physical panel or the user's blue-screen observation.
Static inspection finds the Window raster path copies the already resolved
32-bit graphics pixels into a top-down 32-bit `BI_RGB` DIB; it contains no
separate RGB/BGR swapping branch. This narrows the investigation but cannot
replace sampling the actual native client area at the reported screen.

The oracle was extracted only into ignored task scratch from the same lawful
BYOB TFT firmware using the external reference extractor, and has SHA-256
`7128F7C5D4907A8C7F1D7B8C0D64F8617CDA43464F36599D21ED7BBCCAB8FE8B`.
It is compared against guest-produced data and is never injected into guest
memory or committed.

## Corrective finding: x86 Release compiler flags

Inspection of the actual Ninja compile command found that the stable x86
Release tree had `CMAKE_C_FLAGS_RELEASE` set to the empty string. Its CPU
objects therefore used the GCC default `-O0`, although the current x86 preset
specifies `-O3 -DNDEBUG` and the product rule requires optimized Release.
The origin of that cache value has not been established. This materially
invalidates S70's x86 optimized-artifact qualification and explains at least
part of the observed x86 throughput difference; the previously recorded
functional observations remain evidence for that unoptimized build only.
The x64 tree's actual command includes `-O3 -DNDEBUG` and is unaffected.

The in-flight unoptimized x86 replay was stopped after identifying its exact
owned process; it is not counted as a failed or passing qualification. S71
corrects the existing x86 tree and rechecks compiler commands, affected
receivers, dual-width products and the true x86 firmware path. A lightweight
NXVM Release configuration check will prevent silently using an unoptimized
Release graph again; it must not change Lib or Emulator.

The configure guard rejected the original empty flags with
`NXVM Release requires optimized C compiler flags`. The existing x86 tree was
then configured with `-O3 -DNDEBUG`, and the real Ninja command for the CPU
confirmed both flags and the repository ccache launcher; all 122 C compilation
commands in the PC110 diagnostic dependency graph contained `-O3` with no
`-O0` override. The full x86 build completed. Its X86-NXVM unit selection
passed 233/233 and its non-X86-NXVM unit selection passed 436/436, covering
669 distinct unit tests. The stopped unoptimized replay log SHA-256 is
`E776FA69BF476AB737F289107348277B36F3779C73144C601301327FF89CC266`.

All four x86 PC artifacts have since been rebuilt with the optimized graph;
their INI identity and boot receiver rows pass. The optimized DeskPro boot
row completed in 189.12 seconds under the temporary 480-second bound, versus
370.32 seconds for the unoptimized 480-second run. Its owner-approved
300-second bound has been restored and the formal CTest rerun passed in
186.10 seconds. Its log SHA-256 is
`7DAF69FC6136F723AE1DD98AE255984D3A1342D2A0E25475DA90B83A5DDDBC7B`;
neither the earlier 300-second failure nor the baseline control is erased.
The x86 X86-NXVM and remaining-unit log SHA-256 values are respectively
`A6E5A9C817E4E126980E3339696F11699B98373D453BC067C5C34742CCB27372`
and `9C1104B34B16F7A7A69B14816F5AB8F68F67985E922850710C438E2064C6330A`.
The four-profile receiving matrix log SHA-256 is
`258567A566B199B7C4BBE9587FBCB027F5E24CF5C0C7BDA634206AB97C536F3E`.
All eight PC file identities and hashes are recorded in the
[S71 artifact inventory](t580-s71-artifacts.json); the x64 files were not
rebuilt by this x86-only correction.

The optimized x86 registered firmware suite passed both FDC reset and ordinary
boot handoff in 65.17 and 355.89 seconds respectively; its output SHA-256 is
`8FD77BA9B5D9B45A9ABC93FCDE8AFDF058A55658A98CA66D2F7718DF0DCC2BC9`.
That run used S70's 1080-second x86 handoff containment. Because 355.89
seconds leaves negligible margin under the former 360-second limit, the
final x86-only containment is 600 seconds; no guest behavior or x64 budget
changes. Reconfiguration of the stable x86 tree and CTest's JSON inventory
confirmed that final registration. The passing run above used the previous
1080-second registration; the identical case was not run again under 600.

The optimized x86 post-Setup replay completed with process exit 0. It observed
the second firmware suspend at `00039333`, delivered the real F12 input,
verified the saved CPU image and RSM, and reached INT 19h at `f000:e6f2`.
Its final result recorded eight expected exceptions and no Core fault; the
log SHA-256 is
`13465F72F06DA8809783CECCB6E1ADECF11CF532EE6B8EAF3FA4B3D30469C8E7`.
This establishes the firmware handoff, not a successful DOS or other guest
boot. CMOS persistence, both panel variants, desktop presentation and actual
bootable media remain T580 work.
