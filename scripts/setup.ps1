$ErrorActionPreference = "Stop"

$RootPath = (Get-Item "$PSScriptRoot\..").FullName

# Initialize and update submodules
Write-Host "Initializing and updating Git submodules..." -ForegroundColor Cyan
git submodule update --init --recursive

# Windows: Install WebView2 NuGet package for JUCE plugins that use NEEDS_WEBVIEW2
if ($IsWindows -or ($env:OS -eq "Windows_NT")) {
    $packagesDir = Join-Path $RootPath "_tools\packages"
    $webview2Package = "Microsoft.Web.WebView2"
    $webview2Version = "1.0.3485.44"
    $existingPackage = Get-ChildItem -Path $packagesDir -Filter "*$webview2Package*" -Directory -ErrorAction SilentlyContinue | Select-Object -First 1

    if ($existingPackage) {
        Write-Host "WebView2 package found: $($existingPackage.Name)" -ForegroundColor DarkGray
    } else {
        Write-Host "Installing WebView2 NuGet package for JUCE build..." -ForegroundColor Cyan
        $nugetExe = Join-Path $RootPath "_tools\nuget.exe"
        $nugetUrl = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"

        if (-not (Test-Path $nugetExe)) {
            $toolsDir = Split-Path $nugetExe -Parent
            if (-not (Test-Path $toolsDir)) { New-Item -ItemType Directory -Path $toolsDir -Force | Out-Null }
            Invoke-WebRequest -Uri $nugetUrl -OutFile $nugetExe -UseBasicParsing
        }

        New-Item -ItemType Directory -Path $packagesDir -Force | Out-Null
        & $nugetExe install $webview2Package -Version $webview2Version -OutputDirectory $packagesDir -NonInteractive
        if ($LASTEXITCODE -ne 0) { throw "NuGet install failed" }
        Write-Host "WebView2 package installed." -ForegroundColor Green
    }
}

Write-Host "Setup Complete." -ForegroundColor Green
