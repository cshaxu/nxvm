param(
    [string]$ManifestPath = "docs/etc/cpu-timing/t435-s2-80386-timing-manifest.json",
    [string]$InventoryPath = "docs/etc/cpu-timing/t435-s8-80386-decoder-inventory.json",
    [string]$LedgerPath = "docs/etc/cpu-timing/t435-s1-80386-ledger.md"
)
$ErrorActionPreference = 'Stop'
function Expand-Template($t) {
  $r=@([pscustomobject]@{key_id=[string]$t.id;axes=@{}})
  foreach($p in $t.PSObject.Properties){
    if($p.Name -in @('id','level','source_rule','route','status','batch','test') -or $p.Value -isnot [array]){continue}
    $n=@();foreach($x in $r){foreach($v in $p.Value){$a=@{};foreach($q in $x.axes.Keys){$a[$q]=$x.axes[$q]};$a[$p.Name]=[string]$v;$n += [pscustomobject]@{key_id=$x.key_id.Replace('{'+$p.Name+'}',[string]$v);axes=$a}}}
    $r=$n
  }
  return $r
}
function Test-Mask($mask,[int]$value){$b=[Convert]::ToByte($mask.Substring(([math]::Floor($value/8)*2),2),16);($b -band (1 -shl ($value%8))) -ne 0}
function Test-Recipe($i,$r){$p=if($null -ne $r.escaped){$i.accepted_0f_modrm_masks.PSObject.Properties[("{0:X2}" -f $r.escaped)]}else{$i.accepted_modrm_masks.PSObject.Properties[("{0:X2}" -f $r.opcode)]};$null -ne $p -and (Test-Mask ([string]$p.Value) $r.modrm)}
function Recipe($key,$a){
  $r=[ordered]@{opcode=$null;modrm=0xc0;escaped=$null};$alu=@{ADD=0;OR=8;ADC=16;SBB=24;AND=32;SUB=40;XOR=48};$g2=@{ROL=0;ROR=1;RCL=2;RCR=3;SHL=4;SHR=5;SAR=7}
  if($key -match '^I386-(ADJ|FLAG)-'){$r.opcode=@{AAA=0x37;AAS=0x3f;DAA=0x27;DAS=0x2f;AAD=0xd5;AAM=0xd4;CBW=0x98;CWDE=0x98;CWD=0x99;CDQ=0x99;CLC=0xf8;CLD=0xfc;CLI=0xfa;CMC=0xf5;LAHF=0x9f;NOP=0x90;SAHF=0x9e;STC=0xf9;STD=0xfd;STI=0xfb}[$a.op]}
  elseif($key -match '^I386-ALU-(ADC|ADD|AND|OR|SBB|SUB|XOR)-'){$b=$alu[$a.op];$e=@('ADD','OR','ADC','SBB','AND','SUB','XOR').IndexOf($a.op);switch($a.form){'RR'{$r.opcode=$b+2;$r.modrm=0xc0}'RM'{$r.opcode=$b+2;$r.modrm=0}'MR'{$r.opcode=$b;$r.modrm=0}'AI'{$r.opcode=$b+4}'RMI'{$r.opcode=0x80;$r.modrm=$e -shl 3}}}
  elseif($key -match '^I386-ALU-(CMP|TEST)-'){$r.opcode=if($a.op -eq 'CMP'){0x3a}else{0x84};if($a.form -eq 'MR'){$r.opcode-=2};if($a.form -eq 'AI'){$r.opcode=if($a.op -eq 'CMP'){0x3c}else{0xa8}};if($a.form -eq 'RMI'){$r.opcode=if($a.op -eq 'CMP'){0x80}else{0xf6};$r.modrm=if($a.op -eq 'CMP'){0x38}else{0}}elseif($a.form -eq 'RM'){$r.modrm=0}elseif($a.form -eq 'RR'){$r.modrm=0xc0}}
  elseif($key -match '^I386-(INC|DEC|NEG|NOT|XCHG)-'){$o=$a.op;if($o -in 'INC','DEC'){$r.opcode=if($a.form -eq 'R'){if($o -eq 'INC'){0x40}else{0x48}}else{0xfe;$r.modrm=if($o -eq 'INC'){0}else{8}}}elseif($o -eq 'XCHG'){$r.opcode=0x87;$r.modrm=if($a.form -eq 'R'){0xc0}else{0}}else{$r.opcode=0xf6;$r.modrm=if($a.form -eq 'R'){if($o -eq 'NEG'){0xd8}else{0xd0}}else{if($o -eq 'NEG'){0x18}else{0x10}}}}
  elseif($key -match '^I386-(MUL|IMUL1|DIV|IDIV)-'){$r.opcode=if($a.form -match '8'){0xf6}else{0xf7};$e=@{MUL=4;IMUL1=5;DIV=6;IDIV=7}[$a.op];$r.modrm=if($a.form -match '^R'){0xc0 -bor ($e -shl 3)}else{$e -shl 3}}
  elseif($key -match '^I386-IMUL2-'){$r.escaped=0xaf;$r.opcode=$null;$r.modrm=if($a.form -match '^RR'){0xc0}else{0}}
  elseif($key -match '^I386-IMUL3-'){$r.opcode=if($a.form -match 'IMM8'){0x6b}else{0x69};$r.modrm=if($a.form -match '^RR'){0xc0}else{0}}
  elseif($key -match '^I386-(ROL|ROR|RCL|RCR|SHL|SHR|SAR|SHLD|SHRD)-'){$r.modrm=$g2[$a.op] -shl 3;if($a.op -in 'SHLD','SHRD'){$r.escaped=if($a.op -eq 'SHLD'){0xa4}else{0xac};$r.opcode=$null}else{$r.opcode=if($a.form -match 'CL'){0xd2}elseif($a.form -match 'IMM'){0xc0}else{0xd0}}}
  elseif($key -match '^I386-MOV-'){$f=$a.form;$r.opcode=@{'RR'=0x8a;'RM'=0x8a;'MR'=0x88;'RI'=0xb0;'MI'=0xc6;'MOFFS-R'=0xa0;'MOFFS-W'=0xa2;'SREG-LOAD'=0x8e;'SREG-STORE'=0x8c;MOVSX=0x0f;MOVZX=0x0f;LEA=0x8d;LDS=0xc5;LES=0xc4;LFS=0x0f;LGS=0x0f;LSS=0x0f}[$f];if($f -in 'MOVSX','MOVZX','LFS','LGS','LSS'){$r.escaped=@{MOVSX=0xbe;MOVZX=0xb6;LFS=0xb4;LGS=0xb5;LSS=0xb2}[$f];$r.opcode=$null};if($f -match '^(R32-CR|CR|R32-DR|DR|R32-TR|TR)'){$r.escaped=if($f -match 'CR'){if($f -match '^R'){0x20}else{0x22}}elseif($f -match 'DR'){if($f -match '^R'){0x21}else{0x23}}elseif($f -match '^R'){0x24}else{0x26};$r.opcode=$null;$r.modrm=if($f -match 'CR2'){0xd0}elseif($f -match 'CR3'){0xd8}elseif($f -match 'DR6|TR6'){0xf0}else{0xc0}}elseif($f -in 'RM','MR','MI','LEA','LDS','LES','LFS','LGS','LSS','SREG-STORE'){$r.modrm=0}}
  elseif($key -match '^I386-(CALL|JMP|RET|IRET|INT|INTO)'){$r.opcode=if($key -match '^I386-CALL') {if($a.path -match 'NEAR-R'){0xff}elseif($a.path -match 'M$'){ $r.modrm=0x18;0xff}elseif($a.path -match 'NEAR'){0xe8}else{0x9a}}elseif($key -match '^I386-JMP'){if($a.path -match 'NEAR-R'){$r.modrm=0xe0;0xff}elseif($a.path -match 'M$'){$r.modrm=0x28;0xff}elseif($a.path -match 'NEAR'){0xe9}else{0xea}}elseif($key -match '^I386-RET'){if($a.path -match 'NEAR'){if($a.path -match 'IMM'){0xc2}else{0xc3}}else{if($a.path -match 'IMM'){0xca}else{0xcb}}}elseif($key -match '^I386-IRET'){0xcf}elseif($key -match 'INT3'){0xcc}elseif($key -match 'INT-IMM'){0xcd}else{0xce}}
  elseif($key -match '^I386-(IN|OUT)-'){$r.opcode=if($a.op -eq 'IN'){if($a.port -eq 'IMM'){0xe4}else{0xec}}else{if($a.port -eq 'IMM'){0xe6}else{0xee}}}
  elseif($key -match '^I386-(ESC|WAIT|HLT|XLAT)'){$r.opcode=@{ESC=0xd8;WAIT=0x9b;HLT=0xf4;XLAT=0xd7}[($key -split '-')[1]]}
  elseif($key -match '^I386-ARPL'){$r.opcode=0x63;$r.modrm=if($a.form -eq 'R'){0xc0}else{0}}
  elseif($key -match '^I386-BOUND'){$r.opcode=0x62;$r.modrm=0}
  elseif($key -match '^I386-(BSF|BSR)-'){$r.escaped=if($a.op -eq 'BSF'){0xbc}else{0xbd};$r.opcode=$null;$r.modrm=if($a.form -match '^R'){0xc0}else{0}}
  elseif($key -match '^I386-(BT|BTC|BTR|BTS)-'){$r.escaped=@{BT=0xa3;BTC=0xbb;BTR=0xb3;BTS=0xab}[$a.op];$r.opcode=$null;$r.modrm=if($a.form -match 'I') { @{BT=0x20;BTC=0x38;BTR=0x30;BTS=0x28}[$a.op] } elseif($a.form -match '^R'){0xc0}else{0};if($a.form -match 'I'){$r.escaped=0xba}}
  elseif($key -match '^I386-(STRING|REP)-'){$o=$a.op -replace '-REPE|-REPNE','';$r.opcode=@{MOVS=0xa4;CMPS=0xa6;STOS=0xaa;LODS=0xac;SCAS=0xae;INS=0x6c;OUTS=0x6e}[$o];if($a.width -ne 'B'){$r.opcode++}}
  elseif($key -match '^I386-SYSTEM-'){$o=if($null -ne $a.op){$a.op}else{($key -split '-')[2]};$r.escaped=@{LAR=2;LSL=3;VERR=0;VERW=0;LLDT=0;LTR=0;LMSW=1;SLDT=0;SMSW=1;STR=0;LGDT=1;LIDT=1;SGDT=1;SIDT=1;CLTS=6}[$o];$r.opcode=$null;$e=@{VERR=4;VERW=5;LLDT=2;LTR=3;LMSW=6;SLDT=0;SMSW=4;STR=1;LGDT=2;LIDT=3;SGDT=0;SIDT=1}[$o];$r.modrm=if($o -eq 'CLTS'){0}elseif($key -match '-R'){0xc0 -bor ($e -shl 3)}else{$e -shl 3}}
  elseif($key -match '^I386-STACK-'){$r.opcode=@{'PUSH-R'=0x50;'PUSH-M'=0xff;'PUSH-IMM'=0x68;'PUSH-SREG'=0x06;'POP-R'=0x58;'POP-M'=0x8f;'POP-SREG'=0x07;PUSHA=0x60;PUSHAD=0x60;POPA=0x61;POPAD=0x61;PUSHF=0x9c;PUSHFD=0x9c;POPF=0x9d;POPFD=0x9d;'ENTER-L0'=0xc8;'ENTER-L1'=0xc8;'ENTER-LN'=0xc8;LEAVE=0xc9}[$a.form];if($a.form -eq 'PUSH-M'){$r.modrm=0x30}elseif($a.form -eq 'POP-M'){$r.modrm=0}}
  elseif($key -match '^I386-JCC-'){$r.opcode=0x70}
  elseif($key -match '^I386-SETCC-'){$r.escaped=0x90;$r.opcode=$null;$r.modrm=if($a.form -eq 'R8'){0xc0}else{0}}
  if($null -eq $r.opcode -and $null -eq $r.escaped){throw "No decoder recipe for $key"};[pscustomobject]$r
}
foreach($p in @($ManifestPath,$InventoryPath,$LedgerPath)){if(-not(Test-Path $p)){throw "Missing $p"}}
$m=Get-Content -Raw $ManifestPath|ConvertFrom-Json;$i=Get-Content -Raw $InventoryPath|ConvertFrom-Json;$l=Get-Content -Raw $LedgerPath
if($m.profile -ne '80386DX' -or $i.lexeme_opcode_modrm_candidates -ne 63021 -or $i.lexeme_primary_opcode_count -ne 253){throw 'Unexpected 80386 denominator'}
if(($l -split "`r?`n"|Where-Object{$_ -match '^\| ' -and $_ -match 'I386DX-PRM-1990'}).Count -lt 20){throw 'S1 manual locators incomplete'}
$base=@();foreach($t in $m.base_templates){foreach($x in Expand-Template $t){$r=Recipe $x.key_id $x.axes;if(-not(Test-Recipe $i $r)){throw "Decoder rejects $($x.key_id)"};$base += $x}}
if($base.Count -ne 451 -or ($base.key_id|Sort-Object -Unique).Count -ne 451){throw 'Base key mismatch'}
$primary=@($i.accepted_modrm_masks.PSObject.Properties.Name);foreach($hex in $primary){$v=[Convert]::ToByte($hex,16);if($hex -in @('26','2E','36','3E','64','65','66','67','F0','F2','F3','0F')){continue};if($v -ge 0xd8 -and $v -le 0xdf){continue};if(-not (($v -le 0x3f) -or ($v -ge 0x40 -and $v -le 0x63) -or ($v -ge 0x68 -and $v -le 0xcf) -or ($v -ge 0xd0 -and $v -le 0xd5) -or $v -eq 0xd7 -or ($v -ge 0xe0 -and $v -le 0xef) -or ($v -ge 0xf4 -and $v -le 0xfd) -or ($v -ge 0xfe -and $v -le 0xff))){throw "Unpartitioned primary opcode $hex"}}
$escaped=@($i.accepted_0f_modrm_masks.PSObject.Properties.Name);$expectedEscaped=@('00','01','02','03','06','20','21','22','23','24','26','80','81','82','83','84','85','86','87','88','89','8A','8B','8C','8D','8E','8F','90','91','92','93','94','95','96','97','98','99','9A','9B','9C','9D','9E','9F','A0','A1','A3','A4','A5','A8','A9','AB','AC','AD','AF','B2','B3','B4','B5','B6','B7','BA','BB','BC','BD','BE','BF');if((Compare-Object $expectedEscaped $escaped)){throw '0F decoder family partition drifted'}
$c=@{};foreach($t in $m.base_templates){foreach($x in Expand-Template $t){$c[$x.key_id]=[string]$t.status}};foreach($s in $m.context_key_sets){$suffix=@($s.id_suffix);if($null -ne $s.phase){$suffix=@($s.phase|ForEach-Object{$s.id_suffix.Replace('{phase}',[string]$_)})};if($null -ne $s.kind){$suffix=@($s.kind|ForEach-Object{$s.id_suffix.Replace('{kind}',[string]$_)})};$rx=[regex]$s.base_selector;foreach($x in $base){if($rx.IsMatch($x.key_id)){foreach($q in $suffix){$c["$($x.key_id)-$q"]=[string]$s.status}}}}
if($c.Count -ne 1413){throw "Canonical mismatch $($c.Count)"}
$canonicalStatus=$c.Values|Group-Object -AsHashTable -AsString;if($canonicalStatus['missing-test'].Count -ne 339 -or $canonicalStatus['missing-input'].Count -ne 1054 -or $canonicalStatus.unallocated.Count -ne 20){throw 'Canonical status accounting drifted'}
$status=@{};foreach($t in $m.base_templates){foreach($x in Expand-Template $t){$status[$t.status]=1+$status[$t.status]}};if($status['missing-test'] -ne 258 -or $status['missing-input'] -ne 192 -or $status.unallocated -ne 1){throw 'Base status accounting drifted'}
"M5:T435:S8:I386-MANUAL-DECODER-PARTITION:OK:base=$($base.Count):primary=$($i.lexeme_primary_opcode_count)"
"M5:T435:S8:I386-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:canonical=$($c.Count)"
"M5:T435:S8:I386-S2-CANONICAL-STATUS-RECONCILED:PASS:missing-test=$($status['missing-test']):missing-input=$($status['missing-input']):unallocated=$($status.unallocated)"
"M5:T435:S8:I386-CANONICAL-STATUS-RECONCILED:PASS:missing-test=$($canonicalStatus['missing-test'].Count):missing-input=$($canonicalStatus['missing-input'].Count):unallocated=$($canonicalStatus.unallocated.Count)"
