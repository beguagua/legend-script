[CmdletBinding()]
param(
  [string]$Binary = "$(Split-Path -Parent $PSScriptRoot)\build\lgnd.exe",
  [switch]$UserPath
)

$ErrorActionPreference = "Stop"
$installRoot = Join-Path ${env:ProgramFiles} "Legend"
$bin = Join-Path $installRoot "bin"

if (-not (Test-Path $Binary)) {
  throw "lgnd.exe não encontrado em '$Binary'. Compile primeiro com: cmake --build build --config Release"
}

New-Item -ItemType Directory -Force -Path $bin | Out-Null
Copy-Item -Force $Binary (Join-Path $bin "lgnd.exe")

$scope = if ($UserPath) { "User" } else { "Machine" }
$current = [Environment]::GetEnvironmentVariable("Path", $scope)
$paths = @($current -split ';' | Where-Object { $_ -and $_ -ne $bin })
[Environment]::SetEnvironmentVariable("Path", (($paths + $bin) -join ';'), $scope)

Write-Host "Legend instalada em $bin\lgnd.exe"
Write-Host "Abra um novo terminal e execute: lgnd --version"
