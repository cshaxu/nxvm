# WineVDM Integration Boundary

The classifier recognizes COM, MZ, NE, PE, and unsupported inputs without
executing them. COM and eligible MZ files enter the owned DOS runtime. NE files
may be sent to an explicitly configured external WineVDM executable. PE files
are rejected with a diagnostic or left to the ordinary Windows launcher.

The external adapter must preserve the target path, working directory, and
arguments; return a clear missing-backend diagnostic; and log only non-sensitive
launch metadata. It must not assume WineVDM is installed, modify global file
associations, or copy WineVDM into this repository.
