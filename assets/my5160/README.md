# My5160 Product Binaries

This flat directory contains the latest verified `my5160_<version>_x64.exe`
and `my5160_<version>_x86.exe`, alongside owner-maintained `my5160.ini`.
Version components use underscores; product display capitalization is unchanged.

The executable loads only its adjacent INI. Relative media paths resolve from
that INI directory, not the working directory. Preserve the INI contents and
external owner media when updating binaries; recover replaced versions from Git.
