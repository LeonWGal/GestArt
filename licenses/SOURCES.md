# Corresponding source

The linked library sources are unmodified:

- Qt 6.8.3: https://download.qt.io/official_releases/qt/6.8/6.8.3/single/qt-everywhere-src-6.8.3.tar.xz
- libheif 1.23.6: https://github.com/strukturag/libheif/archive/refs/tags/v1.23.6.zip
- libde265 1.1.3: https://github.com/strukturag/libde265/archive/refs/tags/v1.1.3.zip
- FFmpeg n7.1: https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n7.1.tar.gz
- Qt's FFmpeg build recipe: https://code.qt.io/cgit/qt/qt5.git/tree/coin/provisioning?h=v6.8.3

Codec ZIPs are included in the Windows package under `licenses/sources`.
`tools/build-codecs.ps1` records their SHA-256 checksums and build configuration.
Qt's module-specific third-party notices are available in the Qt source archive
and at https://doc.qt.io/qt-6.8/licenses-used-in-qt.html .

- Qt WebEngine 6.8.3 (including matching Chromium source): https://download.qt.io/official_releases/qt/6.8/6.8.3/submodules/qtwebengine-everywhere-src-6.8.3.tar.xz
- Qt WebChannel 6.8.3: https://download.qt.io/official_releases/qt/6.8/6.8.3/submodules/qtwebchannel-everywhere-src-6.8.3.tar.xz
- Qt Positioning 6.8.3: https://download.qt.io/official_releases/qt/6.8/6.8.3/submodules/qtpositioning-everywhere-src-6.8.3.tar.xz

The Windows WebEngine, WebChannel and Positioning DLLs use unmodified official Qt 6.8.3 MSVC 2022 packages. Official SPDX manifests record their component licenses.
