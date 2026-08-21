param([string]$ManifestPath = "docs/etc/cpu-timing/t435-s2-80286-timing-manifest.json")
$ErrorActionPreference = "Stop"
function Expand-Template($Template) {
    $keys = @($Template.id)
    foreach ($property in $Template.PSObject.Properties) {
        if ($property.Name -in @("id","level","source_rule","route","status","batch","test")) { continue }
        if ($property.Value -isnot [System.Array]) { continue }
        $next = @(); foreach ($key in $keys) { foreach ($value in $property.Value) { $next += $key.Replace("{" + $property.Name + "}", [string]$value) } }; $keys = $next
    }; return $keys
}
if (-not (Test-Path -LiteralPath $ManifestPath)) { throw "Manifest not found: $ManifestPath" }
$manifest = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
if ($manifest.schema -ne "nxvm.cpu-timing-manifest.v1" -or $manifest.profile -ne "80286") { throw "Unexpected manifest schema or profile" }
$seen=@{}; $counts=@{L3=0;L2=0;conforming=0;'wrong-value'=0;unallocated=0;'missing-input'=0;'missing-test'=0}
foreach ($template in $manifest.base_templates) {
  foreach ($field in @("id","level","source_rule","route","status","batch","test")) { if ([string]::IsNullOrWhiteSpace([string]$template.$field)) { throw "Template missing ${field}: $($template.id)" } }
  if ($template.level -ne "L3") { throw "80286 has no accepted non-L3 key: $($template.id)" }
  foreach ($key in (Expand-Template $template)) { if ($seen.ContainsKey($key)) { throw "Duplicate key: $key" }; if ($manifest.status_values -notcontains $template.status) { throw "Unknown status for $key" }; $seen[$key]=$true; ++$counts.L3; ++$counts[[string]$template.status] }
}
if ($seen.Count -ne $manifest.expected.base -or $counts.L3 -ne $manifest.expected.L3 -or $manifest.expected.L2 -ne 0) { throw "Count mismatch: base=$($seen.Count) L3=$($counts.L3) expected=$($manifest.expected.base)/$($manifest.expected.L3)" }
function Expand-ContextSets($sets, $kind) {
    $result=@{}
    foreach ($set in $sets) {
        foreach ($field in @("id_suffix","base_selector","source_rule","route","status","batch","test")) { if ([string]::IsNullOrWhiteSpace([string]$set.$field)) { throw "${kind} missing ${field}: $($set.id_suffix)" } }
        $suffixes=@($set.id_suffix); if ($null -ne $set.phase) { $suffixes=@($set.phase | ForEach-Object { $set.id_suffix.Replace("{phase}",[string]$_) }) }; if ($null -ne $set.bytes) { $suffixes=@($set.bytes | ForEach-Object { $set.id_suffix.Replace("{bytes}",[string]$_) }) }
        $selector=[regex]$set.base_selector; foreach($base in $seen.Keys){if(-not $selector.IsMatch($base)){continue};foreach($suffix in $suffixes){$key="$base-$suffix";if($result.ContainsKey($key)){throw "Duplicate ${kind} key: $key"};$result[$key]=$true}}
    }; return $result
}
$contexts=Expand-ContextSets $manifest.context_key_sets "context"
$combinations=Expand-ContextSets $manifest.combination_context_sets "combination"
foreach($key in $combinations.Keys){if($contexts.ContainsKey($key)){throw "Combination duplicates context: $key"}}
if($contexts.Count -ne $manifest.expected.contexts -or $combinations.Count -ne $manifest.expected.combinations){throw "Context count mismatch: contexts=$($contexts.Count) combinations=$($combinations.Count)"}
"80286 timing manifest: base=$($seen.Count) L3=$($counts.L3) conforming=$($counts.conforming) wrong=$($counts['wrong-value']) unallocated=$($counts.unallocated) missing_input=$($counts['missing-input']) missing_test=$($counts['missing-test']) contexts=$($contexts.Count) combinations=$($combinations.Count)"
