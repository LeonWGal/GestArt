param([string]$Generator='Ninja')
$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
$sourceRoot=Join-Path $projectRoot '.deps/sources'
$prefix=Join-Path $projectRoot '.deps/codecs'
New-Item -ItemType Directory -Path $sourceRoot -Force | Out-Null
$packages=@(
  @{Name='libde265';Version='1.1.3';Hash='2DEC9B2D6C788A4D21580AA61D0D53118BF699CB3D7E1AAEFB4264B5D1C5F494'},
  @{Name='libheif';Version='1.23.6';Hash='18B645EB85FDADAFBD9443C4FBFADB220F8E5CD1EA28CA76C973FF2C90689BDB'}
)
foreach($package in $packages){
  $name=$package.Name;$version=$package.Version
  $archive=Join-Path $sourceRoot "$name-$version.zip"
  if(!(Test-Path -LiteralPath $archive)){Invoke-WebRequest "https://github.com/strukturag/$name/archive/refs/tags/v$version.zip" -OutFile $archive}
  if((Get-FileHash -LiteralPath $archive).Hash -ne $package.Hash){throw "Checksum mismatch: $archive"}
  if(!(Test-Path -LiteralPath (Join-Path $sourceRoot "$name-$version"))){Expand-Archive -LiteralPath $archive -DestinationPath $sourceRoot}
  $build=Join-Path $sourceRoot "build-$name-$version"
  $architecture=if($Generator.StartsWith('Visual Studio')){@('-A','x64')}else{@()}
  $options=if($name -eq 'libde265'){
    @('-DENABLE_DECODER=OFF','-DENABLE_ENCODER=OFF','-DENABLE_SDL=OFF','-DENABLE_AVX2=OFF','-DENABLE_AVX512=OFF')
  }else{
    @('-DENABLE_PLUGIN_LOADING=OFF','-DWITH_LIBDE265=ON','-DWITH_LIBDE265_PLUGIN=OFF',
      '-DWITH_X265=OFF','-DWITH_X264=OFF','-DWITH_AOM_DECODER=OFF','-DWITH_AOM_ENCODER=OFF',
      '-DWITH_OpenH264_DECODER=OFF','-DWITH_EXAMPLES=OFF','-DWITH_GDK_PIXBUF=OFF',
      '-DBUILD_TESTING=OFF','-DWITH_LIBSHARPYUV=OFF','-DBUILD_DOCUMENTATION=OFF')
  }
  cmake -S (Join-Path $sourceRoot "$name-$version") -B $build -G $Generator @architecture "-DCMAKE_INSTALL_PREFIX=$prefix" "-DCMAKE_PREFIX_PATH=$prefix" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON @options
  if($LASTEXITCODE){throw "Configure failed: $name"}
  cmake --build $build --config Release --parallel 4
  if($LASTEXITCODE){throw "Build failed: $name"}
  cmake --install $build --config Release
  if($LASTEXITCODE){throw "Install failed: $name"}
}
