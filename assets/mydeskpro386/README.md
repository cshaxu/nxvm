# MyDeskPro386 Product Binaries

This flat directory contains the latest verified `mydeskpro386_<version>_x64.exe`
and `mydeskpro386_<version>_x86.exe`, alongside owner-maintained `mydeskpro386.ini`.
Version components use underscores; product display capitalization is unchanged.

The executable loads only its adjacent INI. Relative media paths resolve from
that INI directory, not the working directory. Preserve the INI contents and
external owner media when updating binaries; recover replaced versions from Git.
