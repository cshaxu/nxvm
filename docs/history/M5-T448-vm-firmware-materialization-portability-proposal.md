# M5 VM Firmware Materialization Portability And Dead-Code Closure

## Purpose

Promote the audited firmware materialization debt to a bounded candidate:
remove C11 translation-limit violations without changing emitted ROM bytes,
and remove the unused session helper rather than retaining dead code.

## Required Scope

Replace every generated firmware literal exceeding C11's guaranteed source
translation limit with one explicit chunk/capacity materialization route. Prove
byte-for-byte equivalence for every affected ROM, preserve failure ownership,
and remove `vm_session_read_u16` unless one necessary owner-local caller is
established.

## Dependencies And Completion

Completion requires a complete literal inventory, equivalence fixtures or
hashes over emitted bytes, strict source compilation, dead-symbol sweep, and
current gates.

## Non-goals And Stop Conditions

Do not reassemble or alter firmware semantics, import firmware/media, silence
warnings, create an alternative firmware pipeline, or broaden this cleanup to
profile policy. Stop if equivalence cannot be proved from project-owned inputs.
