param([string]$QtRoot = "", [ValidateSet('Debug','Release')][string]$Configuration = 'Release', [switch]$Package,
      [string]$Generator = '', [string]$BuildDirectory = 'build')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
if (!$QtRoot) { $QtRoot = Join-Path $projectRoot '.deps\Qt\6.8.3\msvc2022_64' }
if (!(Test-Path -LiteralPath (Join-Path $QtRoot 'lib\cmake\Qt6'))) { throw 'Qt SDK not found. Pass -QtRoot with your Qt 6.8+ installation.' }
$buildRoot = Join-Path $projectRoot $BuildDirectory
$codecRoot = Join-Path $projectRoot '.deps\codecs'
if(!$Generator){
  if($IsWindows){
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $vsVersion=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion
    $Generator=if($vsVersion.StartsWith('18.')){'Visual Studio 18 2026'}else{'Visual Studio 17 2022'}
  }else{$Generator='Ninja'}
}
$architecture=if($Generator.StartsWith('Visual Studio')){@('-A','x64')}else{@()}
cmake -S $projectRoot -B $buildRoot -G $Generator @architecture "-DCMAKE_PREFIX_PATH=$QtRoot;$codecRoot" "-DCMAKE_BUILD_TYPE=$Configuration" -DBUILD_TESTING=ON
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }
cmake --build $buildRoot --config $Configuration --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
$pathSeparator=[IO.Path]::PathSeparator
$env:PATH="$QtRoot/bin$pathSeparator$codecRoot/bin$pathSeparator$env:PATH"
if(!$IsWindows){$env:LD_LIBRARY_PATH="$QtRoot/lib$pathSeparator$codecRoot/lib$pathSeparator$env:LD_LIBRARY_PATH"}
ctest --test-dir $buildRoot -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
if ($Package) {
  cmake --install $buildRoot --config $Configuration --prefix (Join-Path $projectRoot 'release')
  if ($LASTEXITCODE -ne 0) { throw 'Packaging failed' }
}
