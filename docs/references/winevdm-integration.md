# WineVDM Research Boundary

WineVDM is one candidate external backend for M6 Win16 research. ntvdm64 does
not currently classify, route, or claim support for NE executables.

Any future adapter must remain out of process, preserve target path, working
directory, and arguments, diagnose a missing backend clearly, and avoid global
file association changes. WineVDM is GPL-2.0 and is not copied, linked,
embedded, or bundled without a separate owner-approved licensing decision.
