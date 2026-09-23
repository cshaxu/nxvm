param(
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot,

    [Parameter(Mandatory = $true)]
    [ValidateSet('nxvm', 'mynes')]
    [string]$Product
)

$ErrorActionPreference = 'Stop'

function Require([bool]$Condition, [string]$Message)
{
    if (-not $Condition) {
        throw $Message
    }
}

$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$docs = Join-Path $root 'docs'
$productDocs = Join-Path $docs $Product
$rootRules = Join-Path $docs 'rules'

foreach ($path in @(
    (Join-Path $docs 'README.md'),
    (Join-Path $rootRules 'ARCHITECTURE.md'),
    (Join-Path $rootRules 'CODING.md'),
    (Join-Path $rootRules 'DOCUMENT.md'),
    (Join-Path $rootRules 'EXECUTION.md'),
    (Join-Path $productDocs 'README.md'),
    (Join-Path $productDocs 'design/GOAL.md'),
    (Join-Path $productDocs 'design/ARCHITECTURE.md'),
    (Join-Path $productDocs 'design/CODING.md'),
    (Join-Path $productDocs 'design/UI.md'),
    (Join-Path $productDocs 'design/ROADMAP.md'),
    (Join-Path $productDocs 'states/CURRENT.md'),
    (Join-Path $productDocs 'states/QUEUE.md'),
    (Join-Path $productDocs 'states/TODO.md'),
    (Join-Path $productDocs 'etc/README.md')
)) {
    Require (Test-Path -LiteralPath $path -PathType Leaf) "Required documentation is missing: $path"
}

Require (-not (Test-Path -LiteralPath (Join-Path $productDocs 'rules'))) `
    "Product rules must use the shared docs/rules corpus: $productDocs/rules"

$rootGuide = Get-Content -Raw -LiteralPath (Join-Path $docs 'README.md')
$productGuide = Get-Content -Raw -LiteralPath (Join-Path $productDocs 'README.md')
Require ($rootGuide -match "docs/$Product|$Product/") `
    "Root documentation guide does not identify $Product."
Require ($productGuide -match '\.\./rules/EXECUTION\.md') `
    "Product guide does not link the shared execution rules."
Require ($productGuide -match 'states/CURRENT\.md') `
    "Product guide does not link its current packet."

$queue = Get-Content -Raw -LiteralPath (Join-Path $productDocs 'states/QUEUE.md')
Require (-not ($queue -match '(?m)^\s*[-*]\s+')) `
    "Product queue must use ordered candidates: $Product"

Write-Output "Documentation governance checks passed for $Product."
