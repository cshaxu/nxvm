# Ordinary-RAM Transaction For Staged Runtime Input

 Add a
   stopped/paused copied-value multi-span operation only after its mantle
   staging consumer and full preflight contract are explicit. It validates all
   spans as non-overlapping ordinary RAM before any write, reuses the existing
   checked route, and promises no partial write on validation failure.
