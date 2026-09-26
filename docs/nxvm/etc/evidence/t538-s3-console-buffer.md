# T538 S3: Console Backing Storage

Owner separated this work from the accepted S2 KBC repair. Baseline is S2
1d80f11d8 / 3922622ea; its binaries intentionally retain the original Console
behavior. The S3 candidate was preserved during that delivery, then restored
without changing its production/test bytes. Manifest revision now names S3.

## Contract And Change Review

The Console broker owns one raw-text write path. It grows backing storage to
at least 80 columns and active frame height, without shrinking existing storage.
Visibility may be smaller and scrollable. There is no font adjustment, viewport
resize request, automatic Window fallback or clipping of the supplied frame.
Offscreen rows are written and stale lower rows cleared within frame capacity.
Rejected/ignored backing growth, clipped writes and real cursor/I/O failures
remain errors; visibility alone is not an error.

The query over src/lib/console-broker, src/lib/kvm-console and src/common/ui for
ensure_text_surface, set_console_window_info, srWindow and text-row capacity
identifies one raw-frame capacity owner. Palette application remains before
capacity verification because native metadata can alter geometry. Cooked
restoration is a separate lifecycle responsibility; its partial-startup history
rollback observation stays in the existing NXVM TODO, not claimed repaired here.
No product code, machine state, guest clock or public API is changed.

Actual code/test diff: console.c +10/-35, native display test +5/-24,
I/O contract test +19/-9; total +34/-68, net -34 physical lines from git diff
--numstat, excluding README, manifests, evidence and binaries. Two manifests
and the existing README are updated; the other four corpus roots are unchanged.

## Verification

Existing tests now distinguish full backing storage from visible geometry:
40x13 scrolled viewport with 120x60 storage preserves offscreen row 50's last
cell; 40x13 storage grows to 80x50; rejected and silently ignored growth fail.
Clipped writes and tall-to-short clearing remain covered. Native small-buffer,
scrolled-viewport and existing cooked/raw handoff cases pass.

The earlier private-child font probes in the main T538 ledger remain evidence
for these exact production bytes: previously failing undersized viewports now
enter POST, including an 8x6 maximum visible surface. Font manipulation was in
the diagnostic child only; that temporary wrapper source was removed.
Those bounded probes are not whole-task DOS acceptance.

Restored S3 NXVM units pass 335/335 on each width and external integration
20/20. MyNES passes 132/132 on each width (127 non-desktop plus five serial
desktop cases). All six manifests pass. Eight NXVM 0538 and two MyNES 0043
optimized stripped receivers are rebuilt; architecture checks pass. The last
Model 40 build shells ended without completion, so both final targets were
rerun to successful exit zero before delivery. Four NXVM INI hashes remain
identical to S2. No product source, configuration, snapshot or media changed.

Source: Shared 6a3f3cb25 plus NXVM S2 1d80f11d8. Product builds use the same
Shared source bytes tested above. NXVM SHA-256 by filename (under assets/nxvm):

| EXE | SHA-256 |
| --- | --- |
| nxvm_at_0_5_0538_x64.exe | CC263ED8079771240EE3780A34AC815D53B043B0D16421BE6019FD43F80F0D23 |
| nxvm_at_0_5_0538_x86.exe | 181319C90A504EB003F153ED04466246725CD9BB9710C251B594FAEFC3ECD63A |
| nxvm_xt_0_5_0538_x64.exe | DFA6A6B070CF79E8C605921CF9DD19D04A34B92F80BA9D320B06E6E9F428F24A |
| nxvm_xt_0_5_0538_x86.exe | 9569A19AA541E19E06BB35248D65EABDF1291AE6D76C1CC67B9BB43989976BE0 |
| nxvm_default_0_5_0538_x64.exe | 9089EFC00EEFB75BCDFEA332BFC97398D7E246D708876DBD23ABC3B796F231AC |
| nxvm_default_0_5_0538_x86.exe | EEB891D5A40B4D52741490B11FAFD03F1B498C049EFA1067F3253B25852F97E1 |
| nxvm_model40_0_5_0538_x64.exe | 6B3A4BE1C9BAE169D5BF33CD5BA0928A39FA1D791CDA377307B6018262530EC5 |
| nxvm_model40_0_5_0538_x86.exe | 080E5FF1D9003E93D489AC5E18A08F90D2AB25989B5E47515357BDA143647679 |

MyNES receiver hashes are recorded in its own
[evidence](../../../mynes/etc/evidence/t538-s3-console-receiver.md).

T538's final three-fresh-launch matrix remains a separate completion gate.
