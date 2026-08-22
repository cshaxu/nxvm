param(
    [string]$ManifestPath = "docs/etc/cpu-timing/t435-s2-80186-timing-manifest.json",
    [string]$InventoryPath = "docs/etc/cpu-timing/t435-s6-80186-decoder-inventory.json",
    [string]$LedgerPath = "docs/etc/cpu-timing/t435-s1-80186-ledger.md"
)

$ErrorActionPreference = "Stop"

function Expand-Template {
    param($Template)
    $records = @([pscustomobject]@{ key_id = [string]$Template.id; axes = @{} })
    foreach ($property in $Template.PSObject.Properties) {
        if ($property.Name -in @("id", "level", "source_rule", "route", "status", "batch", "test", "overrides")) { continue }
        if ($property.Value -isnot [System.Array]) { continue }
        $next = @()
        foreach ($record in $records) {
            foreach ($value in $property.Value) {
                $axes = @{}; foreach ($name in $record.axes.Keys) { $axes[$name] = $record.axes[$name] }
                $axes[$property.Name] = [string]$value
                $next += [pscustomobject]@{ key_id = $record.key_id.Replace("{" + $property.Name + "}", [string]$value); axes = $axes }
            }
        }
        $records = $next
    }
    return $records
}

function New-Recipe {
    param([string]$Key, $Axes)
    $opcode = $null; $modrm = 0xc0; $kind = "direct"
    $aluBase = @{ ADD = 0x00; OR = 0x08; ADC = 0x10; SBB = 0x18; AND = 0x20; SUB = 0x28; XOR = 0x30 }
    $condition = @{ JO = 0x70; JNO = 0x71; JB = 0x72; JAE = 0x73; JE = 0x74; JNE = 0x75; JBE = 0x76; JA = 0x77; JS = 0x78; JNS = 0x79; JP = 0x7a; JNP = 0x7b; JL = 0x7c; JGE = 0x7d; JLE = 0x7e; JG = 0x7f }
    $group2 = @{ ROL = 0; ROR = 1; RCL = 2; RCR = 3; SHL = 4; SHR = 5; SAR = 7 }

    if ($Key -match '^I186-ADJ-') {
        $opcode = @{ AAA = 0x37; AAS = 0x3f; AAD = 0xd5; AAM = 0xd4; CBW = 0x98; CWD = 0x99; DAA = 0x27; DAS = 0x2f }[$Axes.op]
    } elseif ($Key -match '^I186-ALU-') {
        $base = $aluBase[$Axes.op]; $extension = [array]::IndexOf(@('ADD','OR','ADC','SBB','AND','SUB','XOR'), $Axes.op)
        $form = if ($null -ne $Axes.form) { $Axes.form } else { 'AI' }
        switch ($form) {
            'RR' { $opcode = $base + 2; $modrm = 0xc0 }
            'RM' { $opcode = $base + 2; $modrm = 0x00 }
            'MR' { $opcode = $base; $modrm = 0x00 }
            'RI' { $opcode = 0x80; $modrm = 0xc0 -bor ($extension -shl 3) }
            'MI' { $opcode = 0x80; $modrm = ($extension -shl 3) }
            'AI' { $opcode = $base + 4 }
        }
    } elseif ($Key -match '^I186-CMP-') {
        $form = if ($null -ne $Axes.form) { $Axes.form } else { 'AI' }
        switch ($form) { 'RR' { $opcode=0x3a; $modrm=0xc0 } 'RM' { $opcode=0x3a; $modrm=0 } 'MR' { $opcode=0x38; $modrm=0 } 'RI' { $opcode=0x80; $modrm=0xf8 } 'MI' { $opcode=0x80; $modrm=0x38 } 'AI' { $opcode=0x3c } }
    } elseif ($Key -match '^I186-TEST-') {
        $form = if ($null -ne $Axes.form) { $Axes.form } else { 'AI' }
        switch ($form) { 'RR' { $opcode=0x84; $modrm=0xc0 } 'RM' { $opcode=0x84; $modrm=0 } 'RI' { $opcode=0xf6; $modrm=0xc0 } 'MI' { $opcode=0xf6; $modrm=0 } 'AI' { $opcode=0xa8 } }
    } elseif ($Key -match '^I186-(INC|DEC)-') {
        $isDec = $Axes.op -eq 'DEC'; switch ($Axes.form) { 'R16' { $opcode = if ($isDec) { 0x48 } else { 0x40 } } 'R8' { $opcode=0xfe; $modrm=if($isDec){0xc8}else{0xc0} } 'M' { $opcode=0xfe; $modrm=if($isDec){0x08}else{0x00} } }
    } elseif ($Key -match '^I186-(NEG|NOT)-') {
        $opcode=0xf6; $extension=if($Axes.op -eq 'NEG'){3}else{2}; $registerBase=if($Axes.form -eq 'R'){0xc0}else{0}; $modrm=$registerBase -bor ($extension -shl 3)
    } elseif ($Key -match '^I186-XCHG-') {
        switch ($Axes.form) { 'AXR' { $opcode=0x91 } 'RR' { $opcode=0x87; $modrm=0xc0 } 'MR' { $opcode=0x87; $modrm=0 } }
    } elseif ($Key -match '^I186-MOV-') {
        switch ($Axes.form) { 'MOFFS-R' { $opcode=0xa0 } 'MOFFS-W' { $opcode=0xa2 } 'RR' { $opcode=0x8a; $modrm=0xc0 } 'RM' { $opcode=0x8a; $modrm=0 } 'MR' { $opcode=0x88; $modrm=0 } 'RI' { $opcode=0xb0 } 'MI' { $opcode=0xc6; $modrm=0 } 'SREG-TO-R' { $opcode=0x8c; $modrm=0xc0 } 'SREG-TO-M' { $opcode=0x8c; $modrm=0 } 'SREG-FROM-R' { $opcode=0x8e; $modrm=0xc0 } 'SREG-FROM-M' { $opcode=0x8e; $modrm=0 } }
    } elseif ($Key -match '^I186-(LEA|LDS|LES)-M$') {
        $opcode=@{LEA=0x8d;LDS=0xc5;LES=0xc4}[$Axes.op]; $modrm=0
    } elseif ($Key -match '^I186-PUSH-') {
        $opcode=@{R=0x50;'SEG-ES'=0x06;'SEG-CS'=0x0e;'SEG-SS'=0x16;'SEG-DS'=0x1e;M=0xff;IMM16=0x68;IMM8=0x6a;F=0x9c}[$Axes.form]; if($Axes.form -eq 'M'){$modrm=0x30}
    } elseif ($Key -match '^I186-POP-') {
        $opcode=@{R=0x58;'SEG-ES'=0x07;'SEG-SS'=0x17;'SEG-DS'=0x1f;M=0x8f;F=0x9d;PUSHA=0x60;POPA=0x61}[$Axes.form]; if($Axes.form -eq 'M'){$modrm=0}
    } elseif ($Key -match '^I186-CALL-') {
        $opcode=@{NEAR=0xe8;FAR=0x9a;RM16=0xff;M1616=0xff}[$Axes.form]; if($Axes.form -eq 'RM16'){$modrm=0xd0}; if($Axes.form -eq 'M1616'){$modrm=0x18}
    } elseif ($Key -match '^I186-JMP-') {
        $opcode=@{SHORT=0xeb;NEAR=0xe9;FAR=0xea;RM16=0xff;M1616=0xff}[$Axes.form]; if($Axes.form -eq 'RM16'){$modrm=0xe0}; if($Axes.form -eq 'M1616'){$modrm=0x28}
    } elseif ($Key -match '^I186-JCC-') { $opcode=$condition[$Axes.condition]; $kind='control-outcome' 
    } elseif ($Key -match '^I186-(JCXZ|LOOP|LOOPE|LOOPNE)-') { $opcode=@{JCXZ=0xe3;LOOP=0xe2;LOOPE=0xe1;LOOPNE=0xe0}[$Axes.op]; $kind='control-outcome'
    } elseif ($Key -match '^I186-RET-') { $opcode=@{NEAR=0xc3;'NEAR-IMM'=0xc2;FAR=0xcb;'FAR-IMM'=0xca;IRET=0xcf}[$Axes.form]
    } elseif ($Key -match '^I186-(INT3|INT-IMM|INTO-TAKEN|INTO-NOT|IN-IMM|IN-DX|OUT-IMM|OUT-DX|ESC-R|ESC-M|WAIT|HLT)$') { $opcode=@{INT3=0xcc;'INT-IMM'=0xcd;'INTO-TAKEN'=0xce;'INTO-NOT'=0xce;'IN-IMM'=0xe4;'IN-DX'=0xec;'OUT-IMM'=0xe6;'OUT-DX'=0xee;'ESC-R'=0xd8;'ESC-M'=0xd8;WAIT=0x9b;HLT=0xf4}[$Axes.op]; if($Axes.op -eq 'ESC-M'){$modrm=0}
    } elseif ($Key -eq 'I186-XLAT') { $opcode=0xd7
    } elseif ($Key -match '^I186-(MUL|IMUL|IDIV|DIV)-(?!IMM)') { $operation=$Matches[1]; $opcode=if($Axes.form -match '16') {0xf7}else{0xf6}; $extensions=@{MUL=4;IMUL=5;DIV=6;IDIV=7}; $extension=$extensions[$operation]; $registerBase=if($Axes.form -match '^R'){0xc0}else{0}; $modrm=$registerBase -bor ($extension -shl 3)
    } elseif ($Key -match '^I186-IMUL-IMM-') { $opcode=if($Axes.form -eq 'IMM8'){0x6b}else{0x69}; $modrm=0xc0
    } elseif ($Key -match '^I186-(ROL|ROR|RCL|RCR|SHL|SHR|SAR)-') { $extension=$group2[$Axes.op]; switch($Axes.form) { 'R1' {$opcode=0xd0;$modrm=0xc0-bor($extension-shl 3)} 'RCL' {$opcode=0xd2;$modrm=0xc0-bor($extension-shl 3)} 'M1' {$opcode=0xd0;$modrm=($extension-shl 3)} 'MCL' {$opcode=0xd2;$modrm=($extension-shl 3)} 'RIMM8' {$opcode=0xc0;$modrm=0xc0-bor($extension-shl 3)} 'MIMM8' {$opcode=0xc0;$modrm=($extension-shl 3)} }
    } elseif ($Key -match '^I186-STRING-') { $opcode=@{MOVS=0xa4;CMPS=0xa6;STOS=0xaa;LODS=0xac;SCAS=0xae;INS=0x6c;OUTS=0x6e}[$Axes.op]; if($Axes.width -eq 'W'){$opcode++}
    } elseif ($Key -match '^I186-REP-') { $base=$Axes.op -replace '-REPE|-REPNE',''; $opcode=@{MOVS=0xa4;CMPS=0xa6;STOS=0xaa;LODS=0xac;SCAS=0xae;INS=0x6c;OUTS=0x6e}[$base]; if($Axes.width -eq 'W'){$opcode++}; $kind='repeat-prefix'
    } elseif ($Key -eq 'I186-BOUND') { $opcode=0x62;$modrm=0
    } elseif ($Key -match '^I186-(ENTER-L0|ENTER-L1|ENTER-LN|LEAVE)$') { $opcode=if($Axes.op -eq 'LEAVE'){0xc9}else{0xc8}; if($Axes.op -ne 'LEAVE'){$kind='enter-level'}
    } elseif ($Key -match '^I186-FLAG-') { $opcode=@{CLC=0xf8;CLD=0xfc;CLI=0xfa;CMC=0xf5;STC=0xf9;STD=0xfd;STI=0xfb;LAHF=0x9f;SAHF=0x9e;NOP=0x90}[$Axes.op] }
    if ($null -eq $opcode) { throw "No decoder recipe for $Key" }
    [pscustomobject]@{ opcode=[int]$opcode; modrm=[int]$modrm; kind=$kind }
}

function Test-InventoryPair {
    param($Inventory, [int]$Opcode, [int]$Modrm)
    $mask = [string]$Inventory.accepted_modrm_masks.PSObject.Properties[("{0:X2}" -f $Opcode)].Value
    if ($mask.Length -ne 64) { return $false }
    $byte = [Convert]::ToByte($mask.Substring(([math]::Floor($Modrm / 8) * 2), 2), 16)
    return ($byte -band (1 -shl ($Modrm % 8))) -ne 0
}

function Test-PrimaryPartition { param([int]$Opcode)
    if ($Opcode -in @(0x26,0x2e,0x36,0x3e,0xf2,0xf3)) { return 'prefix' }
    if ($Opcode -eq 0xf0) { return 'semantic-prefix' }
    if ($Opcode -ge 0xd8 -and $Opcode -le 0xdf) { return 'esc' }
    if ($Opcode -eq 0x0f -or ($Opcode -ge 0x63 -and $Opcode -le 0x67) -or $Opcode -eq 0xf1) { return 'reject' }
    return 'retirement'
}

function Get-RetirementFamily { param([int]$Opcode)
    if (($Opcode -ge 0x00 -and $Opcode -le 0x05) -or ($Opcode -ge 0x08 -and $Opcode -le 0x0d) -or
            ($Opcode -ge 0x10 -and $Opcode -le 0x15) -or ($Opcode -ge 0x18 -and $Opcode -le 0x1d) -or
            ($Opcode -ge 0x20 -and $Opcode -le 0x25) -or ($Opcode -ge 0x28 -and $Opcode -le 0x2d) -or
            ($Opcode -ge 0x30 -and $Opcode -le 0x35)) { return 'ALU' }
    if (($Opcode -ge 0x38 -and $Opcode -le 0x3d)) { return 'CMP' }
    if ($Opcode -in @(0x06,0x0e,0x16,0x1e,0x07,0x17,0x1f,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,0x60,0x61,0x68,0x6a,0x8f,0x9c,0x9d)) { return 'STACK' }
    if ($Opcode -in @(0x27,0x2f,0x37,0x3f,0x98,0x99)) { return 'ADJUST' }
    if (($Opcode -ge 0x40 -and $Opcode -le 0x4f) -or $Opcode -eq 0xfe) { return 'UNARY' }
    if (($Opcode -ge 0x70 -and $Opcode -le 0x7f) -or ($Opcode -ge 0xe0 -and $Opcode -le 0xe3)) { return 'BRANCH' }
    if (($Opcode -ge 0x80 -and $Opcode -le 0x83) -or $Opcode -in @(0x84,0x85,0xa8,0xa9)) { return 'ALU-TEST' }
    if (($Opcode -ge 0x86 -and $Opcode -le 0x8e) -or ($Opcode -ge 0x90 -and $Opcode -le 0x97) -or
            ($Opcode -ge 0xa0 -and $Opcode -le 0xa3) -or ($Opcode -ge 0xb0 -and $Opcode -le 0xbf) -or
            $Opcode -in @(0xc4,0xc5,0xc6,0xc7)) { return 'DATA' }
    if ($Opcode -in @(0x62,0x69,0x6b,0xc0,0xc1,0xc8,0xc9)) { return '80186-ADDITION' }
    if ($Opcode -in @(0x9a,0xe8,0xe9,0xea,0xeb,0xff)) { return 'CONTROL' }
    if ($Opcode -in @(0x9b,0x9e,0x9f,0xf4,0xf5,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd)) { return 'STATE' }
    if (($Opcode -ge 0xa4 -and $Opcode -le 0xa7) -or ($Opcode -ge 0xaa -and $Opcode -le 0xaf) -or ($Opcode -ge 0x6c -and $Opcode -le 0x6f)) { return 'STRING-IO' }
    if ($Opcode -in @(0xc2,0xc3,0xca,0xcb,0xcc,0xcd,0xce,0xcf)) { return 'RETURN-INTERRUPT' }
    if (($Opcode -ge 0xd0 -and $Opcode -le 0xd7)) { return 'GROUP2-XLAT' }
    if (($Opcode -ge 0xe4 -and $Opcode -le 0xef)) { return 'PORT-IO' }
    if ($Opcode -in @(0xf6,0xf7)) { return 'GROUP3' }
    return $null
}

foreach ($path in @($ManifestPath, $InventoryPath, $LedgerPath)) { if (-not (Test-Path -LiteralPath $path)) { throw "Required input not found: $path" } }
$manifest = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
$inventory = Get-Content -Raw -LiteralPath $InventoryPath | ConvertFrom-Json
$ledger = Get-Content -Raw -LiteralPath $LedgerPath
if ($manifest.profile -ne '80186' -or $inventory.lexeme_primary_opcode_count -ne 247 -or $inventory.lexeme_opcode_modrm_candidates -ne 61530) { throw 'Unexpected 80186 manifest or decoder inventory denominator' }
if ($ledger -notmatch 'Every 80186 successful-retirement form has an Intel Table 2-9 primary' -or
        $ledger -notmatch 'M5:T435:S1:80186-FORM-LEDGER:OK') { throw 'S1 80186 manual coverage assertion is missing' }
$ledgerLines = @($ledger -split "`r?`n")
$ledgerHeader = [array]::IndexOf($ledgerLines, '| families and every accepted form/context | 80186 manual result | class | proposed result | primary-manual locator |')
if ($ledgerHeader -lt 0) { throw 'S1 primary-manual locator table is missing' }
$ledgerRows = @()
for ($index = $ledgerHeader + 2; $index -lt $ledgerLines.Count -and $ledgerLines[$index].StartsWith('|'); ++$index) { $ledgerRows += $ledgerLines[$index] }
if ($ledgerRows.Count -ne 23) { throw "Unexpected S1 manual row count: $($ledgerRows.Count)" }
foreach ($row in $ledgerRows) {
    $cells = @($row.Split('|'))
    if ($cells.Count -ne 7 -or [string]::IsNullOrWhiteSpace($cells[5]) -or $cells[5] -notmatch 'I186-UM-1985, Table 2-9') { throw "S1 row lacks a primary manual locator: $row" }
}

$keys = @()
foreach ($template in $manifest.base_templates) {
    foreach ($expanded in (Expand-Template $template)) {
        if ([string]::IsNullOrWhiteSpace([string]$template.source_rule) -or $template.source_rule -notmatch 'Table (1-16|2-9)') { throw "No manual rule for $($expanded.key_id)" }
        $recipe = New-Recipe $expanded.key_id $expanded.axes
        if (-not (Test-InventoryPair $inventory $recipe.opcode $recipe.modrm)) { throw "Current lexical decoder rejects recipe for $($expanded.key_id): $($recipe.opcode.ToString('X2'))/$($recipe.modrm.ToString('X2'))" }
        $status = if ($null -ne $template.overrides -and $null -ne $template.overrides.$($expanded.key_id)) { $template.overrides.$($expanded.key_id) } else { $template.status }
        $keys += [pscustomobject]@{ key_id=$expanded.key_id; recipe=$recipe; status=$status; source_rule=$template.source_rule; route=$template.route }
    }
}
if ($keys.Count -ne 279 -or ($keys.key_id | Sort-Object -Unique).Count -ne 279) { throw "Base-key expansion mismatch: $($keys.Count)" }
$canonical = @{}
foreach ($key in $keys) { $canonical[$key.key_id] = [pscustomobject]@{ status=$key.status; source_rule=$key.source_rule; route=$key.route } }
foreach ($setGroup in @($manifest.context_key_sets, $manifest.combination_context_sets)) {
    foreach ($set in $setGroup) {
        foreach ($field in @('id_suffix', 'base_selector', 'source_rule', 'route', 'status', 'batch', 'test')) {
            if ([string]::IsNullOrWhiteSpace([string]$set.$field)) { throw "Context set missing ${field}: $($set.id_suffix)" }
        }
        $suffixes = @($set.id_suffix)
        if ($null -ne $set.phase) { $suffixes = @($set.phase | ForEach-Object { $set.id_suffix.Replace('{phase}', [string]$_) }) }
        $selector = [regex]$set.base_selector
        foreach ($base in $keys) {
            if (-not $selector.IsMatch($base.key_id)) { continue }
            foreach ($suffix in $suffixes) {
                $contextKey = "$($base.key_id)-$suffix"
                if ($canonical.ContainsKey($contextKey)) { throw "Duplicate canonical context key: $contextKey" }
                if ($manifest.status_values -notcontains [string]$set.status) { throw "Invalid context status for $contextKey" }
                $canonical[$contextKey] = [pscustomobject]@{ status=$set.status; source_rule=$set.source_rule; route=$set.route }
            }
        }
    }
}
if ($canonical.Count -ne 603) { throw "Canonical S2 key count mismatch: $($canonical.Count)" }
foreach ($entry in $canonical.GetEnumerator()) {
    if ([string]::IsNullOrWhiteSpace([string]$entry.Value.status) -or
            [string]::IsNullOrWhiteSpace([string]$entry.Value.source_rule) -or
            [string]::IsNullOrWhiteSpace([string]$entry.Value.route)) { throw "Incomplete current status for $($entry.Key)" }
}

$partitionCounts = @{ prefix=0; 'semantic-prefix'=0; esc=0; retirement=0 }
$familySelectors = @{
    ALU='^I186-ALU-'; CMP='^I186-CMP-'; STACK='^I186-(PUSH|POP)-'; ADJUST='^I186-(ADJ|INC|DEC|NEG|NOT|XCHG)-';
    UNARY='^I186-(INC|DEC)-'; BRANCH='^I186-(JCC|JCXZ|LOOP)'; 'ALU-TEST'='^I186-(ALU|CMP|TEST)-';
    DATA='^I186-(MOV|XCHG|LEA|LDS|LES|FLAG-NOP)'; '80186-ADDITION'='^I186-(BOUND|IMUL-IMM|STRING-(INS|OUTS)|REP-(INS|OUTS)|ENTER|LEAVE|PUSH-IMM|POP-(PUSHA|POPA))';
    CONTROL='^I186-(CALL|JMP|PUSH|INC|DEC)-'; STATE='^I186-(FLAG|WAIT|HLT)'; STRING='^I186-(STRING|REP)-'; 'STRING-IO'='^I186-(STRING|REP)-';
    'RETURN-INTERRUPT'='^I186-(RET|INT)'; 'GROUP2-XLAT'='^I186-(ROL|ROR|RCL|RCR|SHL|SHR|SAR|XLAT)';
    'PORT-IO'='^I186-(IN|OUT)-'; GROUP3='^I186-(MUL|IMUL|DIV|IDIV|NEG|NOT)-'
}
foreach ($opcodeText in $inventory.lexeme_primary_opcodes) {
    $opcode = [Convert]::ToInt32($opcodeText, 16)
    $kind = Test-PrimaryPartition $opcode
    if ($kind -eq 'reject') { throw "Rejected primary opcode appears in inventory: $opcodeText" }
    if ($kind -eq 'retirement') {
        $family = Get-RetirementFamily $opcode
        if ($null -eq $family -or $null -eq $familySelectors[$family] -or
                $keys.Where({ $_.key_id -match $familySelectors[$family] }).Count -eq 0) { throw "Unmapped successful primary opcode: $opcodeText" }
    }
    ++$partitionCounts[$kind]
}
if ($partitionCounts.prefix -ne 6 -or $partitionCounts.'semantic-prefix' -ne 0 -or $partitionCounts.esc -ne 8 -or $partitionCounts.retirement -ne 233) { throw "Primary partition mismatch: prefix=$($partitionCounts.prefix) semantic=$($partitionCounts.'semantic-prefix') esc=$($partitionCounts.esc) retirement=$($partitionCounts.retirement)" }
$statusCounts = @{}; foreach($key in $keys){$statusCounts[[string]$key.status] = 1 + [int]$statusCounts[[string]$key.status]}
if ([int]$statusCounts.conforming -ne 0 -or [int]$statusCounts.'wrong-value' -ne 14 -or [int]$statusCounts.unallocated -ne 43 -or [int]$statusCounts.'missing-test' -ne 222) { throw "S2 base status counts are not reconciled: conforming=$($statusCounts.conforming) wrong=$($statusCounts.'wrong-value') unallocated=$($statusCounts.unallocated) missing-test=$($statusCounts.'missing-test')" }
$canonicalStatusCounts = @{}; foreach($entry in $canonical.Values){$canonicalStatusCounts[[string]$entry.status] = 1 + [int]$canonicalStatusCounts[[string]$entry.status]}
"M5:T435:S6:I186-MANUAL-DECODER-PARTITION:OK:247:233:8:6"
"M5:T435:S6:I186-S1-PRIMARY-LOCATORS:PASS:23"
"M5:T435:S6:I186-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:279"
"M5:T435:S6:I186-S2-STATUS-RECONCILED:PASS:0:14:43:222"
"M5:T435:S6:I186-S2-CANONICAL-STATUS-RECONCILED:PASS:603:$($canonicalStatusCounts.'wrong-value'):$($canonicalStatusCounts.unallocated):$($canonicalStatusCounts.'missing-input'):$($canonicalStatusCounts.'missing-test')"
