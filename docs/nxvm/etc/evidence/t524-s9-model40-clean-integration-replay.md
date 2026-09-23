# T524 S9: Model-40 Clean Integration Replay

## Result

The recorded Model-40 red row was reproduced only while a separate
instruction-level diagnostic consumed a host core.  That diagnostic is not a
machine input and must not overlap the wall-clock integration row.  With that
probe stopped, the unchanged external session
`compaq-deskpro-386-model-40-1200k.yaml` reached `installer-running` in
156.37 seconds.

No FDC, CPU, memory, firmware, profile, YAML, or library defect was found.
Consequently S9 makes no production-source change and introduces no
compatibility path.

## Verification

- Focused external Model-40 YAML boot: `installer-running`, 156.37 seconds.
- Complete external integration: 44/44 passing, 420.53 seconds real time.
- Complete repository-only unit suite: 311/311 passing, 18.61 seconds real
  time.

The integration matrix keeps its existing serial resource declaration for the
real boot rows.  Development diagnostics are not a supported concurrent test
input.
