# Microsoft NTVDM Research

**Status: Research only.**

**No implementation commitment. Not part of the default runtime, default build,
or release.**

Microsoft NTVDM is treated as a coupled system, not as a presumed DOS guest
backend. The research question is whether any bounded, high-value, non-invasive
compatibility path exists after host-integration research has established the
relevant Windows architecture.

This directory records component inventory, startup dependencies, host/private
ABI findings, redistributability constraints, and open questions. It does not
contain Microsoft binaries, a component loader, a BOP dispatcher, or runtime
source code.

Formal Microsoft backend work requires a later Go decision meeting all of these
conditions:

1. a finite, testable component boundary;
2. no project redistribution of Microsoft binaries;
3. no default-release or default-runtime dependency;
4. no default security-policy change or system intrusion;
5. no violation of Machine, DOS, or Platform boundaries; and
6. a better ROI than implementing the equivalent owned compatibility feature.
