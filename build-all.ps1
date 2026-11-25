# build-all.ps1 - multi-platform release builder for Geode mods (Windows host)
#
# Copy this file into any mod's root directory (next to mod.json) and run:
#   ./build-all.ps1                # build every platform the mod.json targets, merge into one .geode
#   ./build-all.ps1 -Install       # ...and install the merged package to your profile
#   ./build-all.ps1 -Platforms windows,macos      # override the platform list manually
#
# What it does:
#   1. Reads mod id, required SDK version, and target platforms from mod.json
#      (the "gd" object: win/mac/android/ios; android means both 32 and 64 bit)
#   2. Builds Windows natively with `geode build`
#   3. Builds every other platform in the official geode-sdk docker cross-compile
#      images (runs docker inside WSL - no Docker Desktop needed)
#   4. Merges the per-platform .geode packages into a single multi-platform
#      .geode with `geode package merge`, placed in dist/
#
# Requirements: geode CLI + clang on Windows, WSL with docker engine inside it
# (one-time: wsl -d Ubuntu -- sudo apt-get install docker.io)

param(
    # Platform override; defaults to whatever the mod.json "gd" field targets
    [string[]]$Platforms = @(),

    # CMake build configuration for the Windows build
    [string]$Configuration = "Release",

    # WSL distro that has docker engine installed
    [string]$WslDistro = "Ubuntu",

    # SDK version for the docker image tags; defaults to the mod.json "geode" field
    [string]$SdkVersion = "",

    # Install the merged .geode to the current Geode profile afterwards
    [switch]$Install
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

if (!(Test-Path "mod.json")) { throw "mod.json not found - run this script from a mod's root directory" }
$mod = Get-Content mod.json -Raw | ConvertFrom-Json
if (!$SdkVersion) { $SdkVersion = $mod.geode }
$modId = $mod.id

# derive platforms from the mod.json "gd" field unless overridden
if (!$Platforms -or $Platforms.Count -eq 0) {
    if ($mod.gd -is [string]) {
        $gdKeys = @("win", "mac", "android", "ios")
    }
    else {
        $gdKeys = @($mod.gd.PSObject.Properties.Name)
    }
    $Platforms = foreach ($key in $gdKeys) {
        switch ($key) {
            "win"     { "windows" }
            "mac"     { "macos" }
            "android" { "android32"; "android64" }
            "ios"     { "ios" }
            default   { Write-Host "    skipping unknown gd platform key '$key'" -ForegroundColor Yellow }
        }
    }
}
if (!$Platforms) { throw "no platforms to build - check the mod.json gd field" }

Write-Host "==> Building $modId for: $($Platforms -join ', ') (SDK $SdkVersion)" -ForegroundColor Cyan

# platform -> docker image + cmake preset inside it
$dockerTargets = @{
    "macos"     = @{ image = "macos";   preset = "mac" }
    "android32" = @{ image = "android"; preset = "android32" }
    "android64" = @{ image = "android"; preset = "android64" }
    "ios"       = @{ image = "ios";     preset = "ios" }
}

# How a Windows path looks from inside WSL
function ConvertTo-WslPath([string]$winPath) {
    $p = $winPath -replace '\\', '/'
    if ($p -match '^([A-Za-z]):/(.*)$') {
        return "/mnt/" + $Matches[1].ToLower() + "/" + $Matches[2]
    }
    throw "Cannot convert path to WSL: $winPath"
}

function Invoke-Wsl([string]$cmd) {
    Write-Host "    [wsl] $cmd" -ForegroundColor DarkGray
    # note: no 2>&1 merge - PS 5.1 turns any stderr noise into a terminating
    # error under ErrorActionPreference=Stop. stderr passes through to the
    # console instead; only stdout is captured.
    $out = wsl -d $WslDistro -- bash -c $cmd
    if ($LASTEXITCODE -ne 0) {
        throw "WSL command failed (exit $LASTEXITCODE)"
    }
    return $out
}

New-Item -ItemType Directory -Force dist | Out-Null
$packages = @()

foreach ($platform in $Platforms) {
    if ($platform -eq "windows") {
        Write-Host "==> [windows] building natively" -ForegroundColor Cyan
        # GEODE_DISABLE_PRECOMPILED_HEADERS works around a clang PCH crash on some toolchains
        geode build --ninja --config $Configuration -- -DGEODE_DISABLE_PRECOMPILED_HEADERS=ON
        if ($LASTEXITCODE -ne 0) { throw "Windows build failed" }

        $pkg = Get-ChildItem "build/*.geode" | Select-Object -First 1
        if (!$pkg) { throw "Windows build produced no .geode package" }
        $dest = "dist/$modId.windows.geode"
        Copy-Item $pkg.FullName $dest -Force
        $packages += $dest
        Write-Host "    -> $dest" -ForegroundColor Green
    }
    elseif ($dockerTargets.ContainsKey($platform)) {
        $t = $dockerTargets[$platform]
        $image = "ghcr.io/geode-sdk/geode:$($t.image)-$SdkVersion"
        Write-Host "==> [$platform] building in docker ($image)" -ForegroundColor Cyan

        # the docker-side flow lives in a generated script file - passing it as
        # arguments through windows -> wsl -> docker would be quoting hell.
        # macOS is special: the standard template hardcodes universal binaries
        # (arm64;x86_64), but ld64.lld in the cross toolchain cannot read fat
        # objects and the osxcross ld64 port cannot read llvm archives - so we
        # build thin per-arch and lipo the slices together afterwards.
        $innerScript = @'
set -e
cd /code
if [ "__PLATFORM__" = "macos" ]; then
    # neutralize the template's hardcoded CMAKE_OSX_ARCHITECTURES in this copy
    sed -i 's/set(CMAKE_OSX_ARCHITECTURES/#set(CMAKE_OSX_ARCHITECTURES/' CMakeLists.txt
    for arch in x86_64 arm64; do
        cmake --preset __PRESET__ --presets-file /docker/CMakePresets.json -B "build-mac-$arch" -DCMAKE_OSX_ARCHITECTURES="$arch"
        cmake --build "build-mac-$arch"
    done
    DYLIB=$(basename "$(ls build-mac-x86_64/*.dylib | head -1)")
    mkdir -p uni
    lipo -create "build-mac-x86_64/$DYLIB" "build-mac-arm64/$DYLIB" -output "uni/$DYLIB"
    lipo -info "uni/$DYLIB"
    geode package new /code --binary "/code/uni/$DYLIB" --output /code/result.geode
else
    cmake --preset __PRESET__ --presets-file /docker/CMakePresets.json
    cmake --build --preset __PRESET__ --presets-file /docker/CMakePresets.json
    cp "$(find ./build-* -name '*.geode' -type f | head -1)" /code/result.geode
fi
'@.Replace('__PRESET__', $t.preset).Replace('__PLATFORM__', $platform)

        # persistent per-mod build dir inside WSL: cmake cache + deps survive between runs
        $wslProject = ConvertTo-WslPath (Get-Location).Path
        $buildDir = "`$HOME/.cache/geode-multi-build/$modId-$platform"
        $syncCmd = @(
            "mkdir -p $buildDir",
            # the container writes root-owned files, so reclaim ownership before re-syncing
            "sudo chown -R `$USER:`$USER $buildDir 2>/dev/null || true",
            # sync project into the WSL-native build dir (building on /mnt/c is very slow)
            "tar -C '$wslProject' --exclude='./build*' --exclude='./.git' --exclude='./dist' --exclude='./ci-artifact' -cf - . | tar -C $buildDir -xf -"
        ) -join " && "
        Invoke-Wsl $syncCmd | Out-Null

        # write the inner script locally with exact LF bytes, then copy it into
        # the staging dir - piping strings to native commands in PS 5.1 would
        # re-serialize every line with CRLF and break bash
        $innerScript = $innerScript -replace "`r`n", "`n"
        $tmpScript = Join-Path $env:TEMP "geode-inner-build-$platform.sh"
        [IO.File]::WriteAllText($tmpScript, $innerScript)
        $wslTmp = ConvertTo-WslPath $tmpScript
        Invoke-Wsl "cp '$wslTmp' '$buildDir/inner-build.sh'" | Out-Null

        Invoke-Wsl "sudo docker run --rm -v `"${buildDir}:/code`" -w /code $image bash /code/inner-build.sh" | Out-Null

        $dest = "dist/$modId.$platform.geode"
        $wslDist = ConvertTo-WslPath (Resolve-Path dist).Path
        Invoke-Wsl "cp '$buildDir/result.geode' '$wslDist/$modId.$platform.geode'" | Out-Null
        if (!(Test-Path $dest)) { throw "Docker build for $platform produced no package" }
        $packages += $dest
        Write-Host "    -> $dest" -ForegroundColor Green
    }
    else {
        throw "Unknown platform '$platform' - valid: windows, macos, android32, android64, ios"
    }
}

# geode package merge merges into the FIRST input file in-place (no output flag),
# so pre-copy the first package to the final name and merge the rest into it
Write-Host "==> merging $($packages.Count) package(s) into one .geode" -ForegroundColor Cyan
$merged = "dist/$modId.geode"
Copy-Item $packages[0] $merged -Force
if ($packages.Count -gt 1) {
    $rest = $packages | Select-Object -Skip 1
    geode package merge $merged @rest
    if ($LASTEXITCODE -ne 0) { throw "geode package merge failed" }
}
# drop the per-platform intermediates
$packages | ForEach-Object { Remove-Item $_ -Force -ErrorAction SilentlyContinue }

Write-Host "==> done: $merged" -ForegroundColor Green

if ($Install) {
    geode package install $merged
    if ($LASTEXITCODE -ne 0) { throw "install failed" }
    Write-Host "==> installed to current profile" -ForegroundColor Green
}
