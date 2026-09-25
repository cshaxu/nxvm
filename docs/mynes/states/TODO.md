# Long-Term Review Ledger

## Governance Debt

| Priority | Debt | Admission path |
| --- | --- | --- |
| P2 | Expose an executable-image-path capability from Lib so App can retain executable-adjacent `mynes.ini` discovery without importing a Win32 SDK declaration. | A future shared-corpus proposal, reviewed and imported through the Lib workflow. |
| P2 | Extend the neutral state-byte reader with a bounded-length/EOF completion fact so Core can reject physical snapshot trailing bytes before committing restore state. | A future Common/Lib state-stream proposal with both receiving products audited. |
| P2 | Shared `library.kvm_window_modal` intermittently observes modal exit before its freeze check (x86 T43 S6 first suite); three unchanged isolated reruns pass. Cause is not established; desktop interaction may affect this native test. Risk: nondeterministic verification, not a proven production regression. | Owner-reviewed Shared test investigation; preserve the failure assertion and separate host interaction from component retirement before changing the test. |
