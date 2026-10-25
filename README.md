# GestArt Ultimate 1.8.0

[English](README.md) · [Русский](README.ru.md)

GestArt Ultimate is a desktop app for artists: choose references, set a drawing
time, and practice with a timer. Use your own images or download sets from
Quickposes, Proko, Pose Library, and SketchDaily. Saved images are available
offline, without a GestArt account.

The **ultimate** branch contains the C++20 and Qt Quick version.
The previous version is preserved in the [old branch](https://github.com/LeonWGal/GestArt/tree/old).

## Changes since GestArt 0.9.8

Ultimate continues the drawing practice app. The timer, breaks, grid, image
effects, and support for your own images remain part of each session.
The main differences from version 0.9.8 are:

| Feature | GestArt 0.9.8 | GestArt Ultimate 1.8.0 |
| --- | --- | --- |
| App foundation | Python and PyQt6 | C++20 and Qt Quick |
| Reference sources | Local folders | Local folders and downloads from Quickposes, Proko, Pose Library, and SketchDaily |
| Organization | Folder browsing and folder statistics | Collection library, previews, and saved sets for individual sessions |
| Websites | Add references from folders | Built-in browser, set selection, full packs, and resumable downloads |
| Practice | Viewing history and folder statistics | Session history, practice calendar, and daily goals |

Version 0.9.8 lists 100 languages; Ultimate contains 22 interface language
catalogs. Some Ultimate translations are machine-generated, and untranslated
strings appear in English.

## Download and start

Prebuilt downloads are for **Windows 10 (1809+) / Windows 11 x64**:

| File | How to start |
| --- | --- |
| [Portable EXE](https://github.com/LeonWGal/GestArt/releases/download/ultimate-v1.8.0/GestArt-Ultimate-1.8.0-windows-x64-portable.exe) | Save it in a writable folder and open it. No installation is needed. |
| [Portable ZIP](https://github.com/LeonWGal/GestArt/releases/download/ultimate-v1.8.0/GestArt-Ultimate-1.8.0-windows-x64-portable.zip) | Extract the entire archive and open `Start.cmd`. |
| [SHA-256](https://github.com/LeonWGal/GestArt/releases/download/ultimate-v1.8.0/SHA256SUMS.txt) | Compare the checksum for your download. |

The portable EXE includes the app and its libraries. On first launch, it
extracts the libraries to a local cache. This launcher requires .NET Framework
4.8 or later. The ZIP starts the app directly and doesn't require .NET.
The prebuilt app doesn't require Python, a Qt SDK, or administrator rights.

The EXE stores settings, history, and downloads in `GestArtUltimate-data`
beside itself. The ZIP stores them in `data` inside the extracted folder.
Keep `bin/GestArtUltimate.exe` together with its libraries.
For details, see the [portable guide](docs/portable.md) in Russian.

## Start with your own images

1. On the home screen, click **Add folder** and select your images.
2. Set the time per reference and the number of studies.
3. Click **Start session**.

The timer starts after the image appears. Space pauses the session, and the
arrow keys change references. Panels hide when the pointer leaves the window;
timer and progress visibility follow your window settings.

## Download a set from a website

1. Click **Download references**.
2. In **Download from website**, choose a website and select sets.
3. Choose the number of images or enable **Download entire pack**.
4. Optional: open **Filters and save folder**.
5. Click **Download selected sets** or **Download entire pack**.
6. In **Saved sets**, click **Set up session**.
7. Set the timer and click **Start session**.

The session uses only the selected set. The list remains available after you
restart the app. You can resume an interrupted download. For a single set,
you can enable **Start a session after downloading**.

Full packs are available for Quickposes, Proko, and Pose Library. SketchDaily
provides random references: choose an image count for this source.
The downloader uses public sets; open the website to access your account
or restricted materials.

![Set selection and download size in the Russian interface](assets/screenshots/downloads.png)

## Open a website inside the app

Choose a website on the **Online viewing** page. Its settings and timer run
inside the built-in browser. When the pointer leaves the window, the top
panel collapses and frees space for the image. Move the pointer back to
restore the controls. **Download references** opens downloads for that website.

To practice with GestArt's timer, grid, mirroring, and statistics, download a
set and start it from **Saved sets**.

## Use session tools

The main keyboard shortcuts are:

| Action | Keys |
| --- | --- |
| Pause / resume | Space |
| Previous / next reference | ← / → |
| Skip without marking complete | S |
| Black-and-white mode | B |
| Flip horizontally / vertically | H / V |
| Rotate 90° | R |
| Grid | G |
| Reset effects | Backspace |
| Fit / 100% zoom | Ctrl+0 / Ctrl+1 |
| Full screen | F11 |
| Settings | Ctrl+, |
| Always on top | A |

The app includes collections, previews, history, a practice calendar, daily
goals, breaks, and countdown sounds. The minimum window size is 360×440.
The Windows build reads JPEG, PNG, BMP, GIF, WebP, TIFF, ICO, SVG, and HEIC/HEIF
with HEVC. GIF images are static; AVIF support depends on the available decoder.

The interface supports 22 languages. Some translations are machine-generated;
untranslated strings use English text.

## Find instructions

The detailed guides are in Russian:

- [Websites, downloads, and saved sets](docs/online-sources.md).
- [Portable builds, data, and updates](docs/portable.md).
- [Sessions, panels, and controls](docs/interface.md).
- [Build from source](docs/build.md).
- [Changelog](CHANGELOG.md).
- [Architecture](docs/architecture.md).
- [Contribute changes](CONTRIBUTING.md).

## Licenses

The app code is distributed under the [MIT license](LICENSE).
Libraries and fonts retain their own licenses; see
[third-party components](THIRD_PARTY_NOTICES.md) and
[library sources](licenses/SOURCES.md).
Downloaded references belong to their authors. Source information is stored
in `sources.json` beside the images.
