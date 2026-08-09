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

M11 T2 inventories the complete possible environment rather than assuming that
Historical system-image components and the command processor are standalone.
Candidate groups are
host runtime, guest DOS files, ROMs, redirectors, debuggers, configuration, WOW,
and Windows host facilities. For each component, record version, architecture,
legal acquisition constraint, hash, role, startup dependency, guest/host class,
public/private ABI dependency, and external-run plausibility.

Map loader, Console, redirection, WOW64, CSRSS, ConHost, registry, services,
private entry points, and version-specific behavior before proposing an
implementation. Open questions are component boundaries, original-host
dependencies, private ABI, non-invasive usefulness, and ROI against owned DOS.

Inputs are owner-provided local BYOB files read only. Evidence records logical
identity, version, architecture, hash, and observations, never local paths or
binary contents. Missing or unclassified components make an experiment
unavailable; they do not authorize fetching, copying, or substitution.
No `ntvdm import` command is promised before the M11 Go decision.

Formal Microsoft backend work requires an M11 Go decision meeting all of these
conditions:

1. a finite, testable component boundary;
2. no project redistribution of Microsoft binaries;
3. no default-release or default-runtime dependency;
4. no default security-policy change or system intrusion;
5. no violation of Machine, DOS, or Platform boundaries; and
6. a better ROI than implementing the equivalent owned compatibility feature.
