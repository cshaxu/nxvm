param(
    [string]$ManifestPath = "docs/etc/cpu-timing/t435-s2-80286-timing-manifest.json",
    [string]$InventoryPath = "docs/etc/cpu-timing/t435-s7-80286-decoder-inventory.json",
    [string]$LedgerPath = "docs/etc/cpu-timing/t435-s1-80286-ledger.md"
)

$ErrorActionPreference = "Stop"

function Expand-Template {
    param($Template)
    $records = @([pscustomobject]@{ key_id = [string]$Template.id; axes = @{} })
    foreach ($property in $Template.PSObject.Properties) {
        if ($property.Name -in @("id", "level", "source_rule", "route", "status", "batch", "test", "overrides")) { continue }
        if ($property.Value -isnot [System.Array]) { continue }
        $next = @()
        foreach ($record in $records) { foreach ($value in $property.Value) {
            $axes = @{}; foreach ($name in $record.axes.Keys) { $axes[$name] = $record.axes[$name] }
            $axes[$property.Name] = [string]$value
            $next += [pscustomobject]@{ key_id = $record.key_id.Replace("{" + $property.Name + "}", [string]$value); axes = $axes }
        } }
        $records = $next
    }
    return $records
}

function Test-Mask {
    param([string]$Mask, [int]$Value)
    if ($Mask.Length -ne 64) { return $false }
    $byte = [Convert]::ToByte($Mask.Substring(([math]::Floor($Value / 8) * 2), 2), 16)
    return ($byte -band (1 -shl ($Value % 8))) -ne 0
}

function Test-Pair {
    param($Inventory, [int]$Opcode, [int]$Modrm)
    $property = $Inventory.accepted_modrm_masks.PSObject.Properties[("{0:X2}" -f $Opcode)]
    return $null -ne $property -and (Test-Mask ([string]$property.Value) $Modrm)
}

function Test-EscapedPair {
    param($Inventory, [int]$Opcode, [int]$Modrm)
    $property = $Inventory.accepted_0f_modrm_masks.PSObject.Properties[("{0:X2}" -f $Opcode)]
    return $null -ne $property -and (Test-Mask ([string]$property.Value) $Modrm)
}

function New-Recipe {
    param([string]$Key, $Axes)
    $opcode = $null; $modrm = 0xc0; $escape = $null
    $alu = @{ ADD=0; OR=8; ADC=16; SBB=24; AND=32; SUB=40; XOR=48 }
    $condition = @{ JO=0x70;JNO=0x71;JB=0x72;JAE=0x73;JE=0x74;JNE=0x75;JBE=0x76;JA=0x77;JS=0x78;JNS=0x79;JP=0x7a;JNP=0x7b;JL=0x7c;JGE=0x7d;JLE=0x7e;JG=0x7f }
    $group2 = @{ ROL=0;ROR=1;RCL=2;RCR=3;SHL=4;SHR=5;SAR=7 }
    if ($Key -match '^I286-ADJ-') { $opcode=@{AAA=0x37;AAS=0x3f;AAD=0xd5;AAM=0xd4;CBW=0x98;CWD=0x99;DAA=0x27;DAS=0x2f}[$Axes.op] }
    elseif ($Key -match '^I286-ALU-') { $base=$alu[$Axes.op]; $ext=@('ADD','OR','ADC','SBB','AND','SUB','XOR').IndexOf($Axes.op); switch($Axes.form) {'RR'{$opcode=$base+2;$modrm=0xc0}'RM'{$opcode=$base+2;$modrm=0}'MR'{$opcode=$base;$modrm=0}'AI'{$opcode=$base+4}'RMI'{$opcode=0x80;$modrm=$ext -shl 3}} }
    elseif ($Key -match '^I286-CMP-') { switch($Axes.form) {'RR'{$opcode=0x3a;$modrm=0xc0}'RM'{$opcode=0x3a;$modrm=0}'MR'{$opcode=0x38;$modrm=0}'AI'{$opcode=0x3c}'RMI'{$opcode=0x80;$modrm=0x38}} }
    elseif ($Key -match '^I286-TEST-') { switch($Axes.form) {'RR'{$opcode=0x84;$modrm=0xc0}'RM'{$opcode=0x84;$modrm=0}'AI'{$opcode=0xa8}'RMI'{$opcode=0xf6;$modrm=0}} }
    elseif ($Key -match '^I286-(INC|DEC|NEG|NOT)-') { $op=$Axes.op; if($op -in @('INC','DEC')){$opcode=if($Axes.form -eq 'R'){if($op -eq 'INC'){0x40}else{0x48}}else{0xfe;$modrm=if($op -eq 'INC'){0}else{8}}}else{$opcode=0xf6;$registerBase=if($Axes.form -eq 'R'){0xc0}else{0};$extension=if($op -eq 'NEG'){3}else{2};$modrm=$registerBase -bor ($extension -shl 3)} }
    elseif ($Key -match '^I286-XCHG-') { switch($Axes.form){'AXR'{$opcode=0x91}'RR'{$opcode=0x87;$modrm=0xc0}'MR'{$opcode=0x87;$modrm=0}} }
    elseif ($Key -match '^I286-(MUL|IMUL|DIV|IDIV)-(?!IMM)') { $opcode=if($Axes.form -match '16'){0xf7}else{0xf6}; $ext=@{MUL=4;IMUL=5;DIV=6;IDIV=7}[$Axes.op]; $registerBase=if($Axes.form -match '^R'){0xc0}else{0}; $modrm=$registerBase -bor ($ext -shl 3) }
    elseif ($Key -match '^I286-IMUL-IMM-') { $opcode=if($Axes.form -match '^IMM8'){0x6b}else{0x69};$modrm=if($Axes.form -match '-R$'){0xc0}else{0} }
    elseif ($Key -match '^I286-(ROL|ROR|RCL|RCR|SHL|SHR|SAR)-') { $ext=$group2[$Axes.op]; switch($Axes.form){'RM1'{$opcode=0xd0}'RMCL'{$opcode=0xd2}'RMIMM8'{$opcode=0xc0}};$modrm=$ext -shl 3 }
    elseif ($Key -match '^I286-MOV-') { switch($Axes.form){'RR'{$opcode=0x8a;$modrm=0xc0}'RM'{$opcode=0x8a;$modrm=0}'MR'{$opcode=0x88;$modrm=0}'RI'{$opcode=0xb0}'MI'{$opcode=0xc6;$modrm=0}'MOFFS-R'{$opcode=0xa0}'MOFFS-W'{$opcode=0xa2}'SREG-STORE'{$opcode=0x8c;$modrm=0}'SREG-LOAD-REAL'{$opcode=0x8e;$modrm=0}'SREG-LOAD-PM'{$opcode=0x8e;$modrm=0}} }
    elseif ($Key -match '^I286-(LEA|LDS|LES)-') {$opcode=@{LEA=0x8d;LDS=0xc5;LES=0xc4}[$Axes.op];$modrm=0}
    elseif ($Key -match '^I286-STACK-') {$opcode=@{'PUSH-R'=0x50;'PUSH-SEG'=0x06;'PUSH-M'=0xff;'PUSH-IMM'=0x68;PUSHA=0x60;PUSHF=0x9c;'POP-R'=0x58;'POP-SEG-REAL'=0x07;'POP-SEG-PM'=0x07;'POP-M'=0x8f;POPA=0x61;POPF=0x9d;LEAVE=0xc9}[$Axes.op];if($Axes.op -eq 'PUSH-M'){$modrm=0x30}elseif($Axes.op -eq 'POP-M'){$modrm=0}}
    elseif ($Key -match '^I286-CALL-') {$opcode=if($Axes.path -match 'NEAR') {if($Axes.path -eq 'NEAR-RM'){0xff}else{0xe8}} elseif($Axes.path -match 'FAR|GATE|TASK'){if($Axes.path -match 'M'){$modrm=0x18;0xff}else{0x9a}};if($Axes.path -eq 'NEAR-RM'){$modrm=0xd0}}
    elseif ($Key -match '^I286-RET-') {$opcode=if($Axes.path -eq 'NEAR'){0xc3}elseif($Axes.path -eq 'NEAR-IMM'){0xc2}elseif($Axes.path -match 'IRET') {0xcf}else{0xcb}}
    elseif ($Key -match '^I286-JCC-') {$opcode=$condition[$Axes.condition]}
    elseif ($Key -match '^I286-(JCXZ|LOOP|LOOPE|LOOPNE|INTO)-') {$opcode=@{JCXZ=0xe3;LOOP=0xe2;LOOPE=0xe1;LOOPNE=0xe0;INTO=0xce}[$Axes.op]}
    elseif ($Key -match '^I286-JMP-') {$opcode=if($Axes.path -eq 'NEAR'){0xe9}elseif($Axes.path -eq 'RM'){ $modrm=0xe0;0xff}elseif($Axes.path -match 'FAR|GATE|TASK'){0xea}}
    elseif ($Key -match '^I286-INTO-') {$opcode=0xce}
    elseif ($Key -match '^I286-INT') {$opcode=if($Key -match 'INT-IMM'){0xcd}else{0xcc}}
    elseif ($Key -match '^I286-IN-IMM') {$opcode=0xe4}
    elseif ($Key -match '^I286-IN-DX') {$opcode=0xec}
    elseif ($Key -match '^I286-OUT-IMM') {$opcode=0xe6}
    elseif ($Key -match '^I286-OUT-DX') {$opcode=0xee}
    elseif ($Key -match '^I286-ENTER-') {$opcode=0xc8}
    elseif ($Key -match '^I286-(ESC|WAIT|HLT|BOUND|ARPL)$') {$opcode=@{ESC=0xd8;WAIT=0x9b;HLT=0xf4;BOUND=0x62;ARPL=0x63}[$Axes.op];if($Axes.op -eq 'BOUND'){$modrm=0}}
    elseif ($Key -eq 'I286-XLAT') {$opcode=0xd7}
    elseif ($Key -match '^I286-STRING-') {$opcode=@{MOVS=0xa4;CMPS=0xa6;STOS=0xaa;LODS=0xac;SCAS=0xae;INS=0x6c;OUTS=0x6e}[$Axes.op];if($Axes.width -eq 'W'){$opcode++}}
    elseif ($Key -match '^I286-REP-') {$base=$Axes.op -replace '-REPE|-REPNE','';$opcode=@{MOVS=0xa4;CMPS=0xa6;STOS=0xaa;LODS=0xac;SCAS=0xae;INS=0x6c;OUTS=0x6e}[$base];if($Axes.width -eq 'W'){$opcode++}}
    elseif ($Key -match '^I286-(CLC|CLD|CMC|STC|STD|STI|CLI|LAHF|SAHF|NOP)$') {$opcode=@{CLC=0xf8;CLD=0xfc;CMC=0xf5;STC=0xf9;STD=0xfd;STI=0xfb;CLI=0xfa;LAHF=0x9f;SAHF=0x9e;NOP=0x90}[$Axes.op]}
    elseif ($Key -match '^I286-SYSTEM-') {$systemOp=if($null -ne $Axes.op){$Axes.op}else{($Key -split '-')[2]};$escape=@{LAR=2;LSL=3;VERR=0;VERW=0;LGDT=1;LIDT=1;SGDT=1;SIDT=1;LLDT=0;LTR=0;LMSW=1;SLDT=0;SMSW=1;STR=0;CLTS=6}[$systemOp];$ext=@{VERR=4;VERW=5;LGDT=2;LIDT=3;SGDT=0;SIDT=1;LLDT=2;LTR=3;LMSW=6;SLDT=0;SMSW=4;STR=1}[$systemOp];if($systemOp -eq 'CLTS'){$modrm=0}else{$registerBase=if($Axes.form -eq 'R'){0xc0}else{0};$modrm=$registerBase -bor ($ext -shl 3)}}
    if ($null -eq $opcode -and $null -eq $escape) { throw "No recipe for $Key" }
    [pscustomobject]@{ opcode=$opcode; modrm=$modrm; escape=$escape }
}

foreach($path in @($ManifestPath,$InventoryPath,$LedgerPath)){if(-not(Test-Path -LiteralPath $path)){throw "Required input not found: $path"}}
$manifest=Get-Content -Raw $ManifestPath|ConvertFrom-Json;$inventory=Get-Content -Raw $InventoryPath|ConvertFrom-Json;$ledger=Get-Content -Raw $LedgerPath
if($manifest.profile -ne '80286' -or $inventory.lexeme_primary_opcode_count -ne 249 -or $inventory.lexeme_opcode_modrm_candidates -ne 61803){throw 'Unexpected 80286 audit denominator'}
if($ledger -notmatch 'I286-PRM-1985, Chapter 8, printed p. 8-109' -or $ledger -notmatch 'M5:T435:S1:80286-FORM-LEDGER:OK'){throw 'S1 XLAT/manual-coverage evidence missing'}
$rows=@($ledger -split "`r?`n"|Where-Object{$_ -match '^\| ' -and $_ -match 'I286-PRM-'});if($rows.Count -lt 19){throw "S1 locator rows incomplete: $($rows.Count)"}
$base=@();foreach($template in $manifest.base_templates){foreach($expanded in Expand-Template $template){if([string]::IsNullOrWhiteSpace([string]$template.source_rule)){throw "Missing source rule for $($expanded.key_id)"};$recipe=New-Recipe $expanded.key_id $expanded.axes;if($null -ne $recipe.escape){if(-not(Test-EscapedPair $inventory $recipe.escape $recipe.modrm)){throw "Decoder rejects escaped recipe $($expanded.key_id)"}}elseif(-not(Test-Pair $inventory $recipe.opcode $recipe.modrm)){throw "Decoder rejects recipe $($expanded.key_id)"};$base += [pscustomobject]@{key_id=$expanded.key_id;status=$template.status;source_rule=$template.source_rule;route=$template.route;opcode=$recipe.opcode;escape=$recipe.escape}}}
if($base.Count -ne 286 -or ($base.key_id|Sort-Object -Unique).Count -ne 286){throw "Base key mismatch: $($base.Count)"}
foreach($primary in $inventory.lexeme_primary_opcodes){if($primary -in @('26','2E','36','3E','F2','F3','0F')){continue};$byte=[Convert]::ToByte($primary,16);if($byte -ge 0xd8 -and $byte -le 0xdf){continue};if(-not (($byte -le 0x3f) -or ($byte -ge 0x40 -and $byte -le 0x63) -or ($byte -ge 0x68 -and $byte -le 0xcf) -or ($byte -ge 0xd0 -and $byte -le 0xd5) -or $byte -eq 0xd7 -or ($byte -ge 0xe0 -and $byte -le 0xef) -or ($byte -ge 0xf4 -and $byte -le 0xfd) -or ($byte -ge 0xfe -and $byte -le 0xff))){throw "Successful primary opcode has no S1 family partition: $primary"}}
$canonical=@{};foreach($key in $base){$canonical[$key.key_id]=$key};foreach($setGroup in @($manifest.context_key_sets,$manifest.combination_context_sets)){foreach($set in $setGroup){foreach($field in @('id_suffix','base_selector','source_rule','route','status','batch','test')){if([string]::IsNullOrWhiteSpace([string]$set.$field)){throw "Context missing $field"}};$suffixes=@($set.id_suffix);if($null -ne $set.phase){$suffixes=@($set.phase|ForEach-Object{$set.id_suffix.Replace('{phase}',[string]$_)})};if($null -ne $set.bytes){$suffixes=@($set.bytes|ForEach-Object{$set.id_suffix.Replace('{bytes}',[string]$_)})};$selector=[regex]$set.base_selector;foreach($key in $base){if($selector.IsMatch($key.key_id)){foreach($suffix in $suffixes){$id="$($key.key_id)-$suffix";if($canonical.ContainsKey($id)){throw "Duplicate canonical key $id"};$canonical[$id]=[pscustomobject]@{status=$set.status;source_rule=$set.source_rule;route=$set.route}}}}}}
if($canonical.Count -ne 807){throw "Canonical key mismatch: $($canonical.Count)"};foreach($entry in $canonical.Values){if([string]::IsNullOrWhiteSpace([string]$entry.status)-or [string]::IsNullOrWhiteSpace([string]$entry.route)){throw 'Incomplete canonical disposition'}}
$counts=@{};foreach($entry in $base){if(-not $counts.ContainsKey($entry.status)){$counts[$entry.status]=0};++$counts[$entry.status]};if($counts.'missing-test' -ne 238 -or $counts.'missing-input' -ne 47 -or $counts.unallocated -ne 1){throw "Unexpected base status disposition"}
"M5:T435:S7:I286-MANUAL-DECODER-PARTITION:OK:base=$($base.Count):primary=$($inventory.lexeme_primary_opcode_count)"
"M5:T435:S7:I286-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:canonical=$($canonical.Count)"
"M5:T435:S7:I286-S2-CANONICAL-STATUS-RECONCILED:PASS:missing-test=$($counts.'missing-test'):missing-input=$($counts.'missing-input'):unallocated=$($counts.unallocated)"
