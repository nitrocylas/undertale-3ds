# Build the Undertale-3DS homebrew. Run from anywhere: powershell -File build.ps1
# Produces undertale-3ds.3dsx in the project root (drop into Azahar, or /3ds/ on the SD card).
$ErrorActionPreference = 'Stop'
$dkp  = 'C:\devkitPro'
$make = "$dkp\msys2\usr\bin\make.exe"
$env:PATH = "$dkp\msys2\usr\bin;$dkp\tools\bin;$dkp\devkitARM\bin;$env:PATH"
Push-Location $PSScriptRoot
try {
    & $make DEVKITPRO=/c/devkitPro DEVKITARM=/c/devkitPro/devkitARM @args
    if ($LASTEXITCODE -ne 0) { throw "make failed ($LASTEXITCODE)" }
    Write-Host "`nOK -> $PSScriptRoot\undertale-3ds.3dsx" -ForegroundColor Green
} finally { Pop-Location }
