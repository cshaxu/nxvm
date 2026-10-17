# Product And Machine Integration

## Objective And Boundary

Connect the existing Emulator Product/Session/UI/Machine contracts to the
MyRaidenII board. Product owns identity, configuration and control policy;
Machine owns the adapter and safe execution boundary; Core owns hardware.
No dependency on another App or PC Product/Debug is permitted.

## Work And Verification

Use one Session provider, command route and execution worker. Implement adjacent
lowercase INI/external ROM paths with transactional validation; retain existing
display/console_control semantics only where their contracts actually support
the graphical product. Freeze coin/start/directions/fire/bomb/service mappings,
focus/reset release behavior, Escape pause policy and Window presentation.
Register standard commands via Emulator; unsupported snapshot/debug features
return the existing honest unsupported result rather than a fake implementation.

## Exit And Stops

Real keyboard-to-guest, frame-to-Window and sound-to-PCM routes work; lifecycle,
missing/mismatched ROM, startup rollback and teardown tests pass in both widths.
Publish latest lowercase dual EXEs with PE/identity/hash checks when production
is delivered. Protected game ROMs remain external, never embedded or committed.
