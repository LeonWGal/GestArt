[English](#english) · [Русский](#русский)

## English

GestArt Ultimate 1.8.0 is the C++20 and Qt Quick version of the drawing practice
app. It continues GestArt 0.9.8 and adds a collection library, online reference
sources, and tools for tracking your practice.

### Changes since GestArt 0.9.8

The timer, breaks, grid, image effects, and support for your own images remain
part of the app. The main Ultimate changes are:

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

### Downloads

For Windows 10 (1809+) / Windows 11 x64, choose one download:

- **Portable EXE** — the app and its libraries in one file. Open the EXE;
  on first launch, it extracts its libraries to a local cache.
  Requires .NET Framework 4.8 or later.
- **Portable ZIP** — extract the entire archive and run `Start.cmd`.
  Doesn't require .NET Framework.
- **SHA256SUMS.txt** — SHA-256 checksums for the EXE and ZIP.

The EXE stores data in `GestArtUltimate-data` beside itself; the ZIP uses `data`
inside the extracted folder. Python and a Qt SDK aren't required to run the app.

### Changes in 1.8.0

- Downloads follow website, set, and size selection with one primary action.
- **Saved sets** remain available after restarting the app. Set the timer before
  starting a saved set; other collections aren't included in the session.
- **Download entire pack** supports Quickposes, Proko, and Pose Library.
  SketchDaily saves the requested number of random references.
- Website and set previews, search, persistent progress, and resumable downloads
  help you choose and save references.
- The browser panel frees space for the image when it hides.
- Windows app and portable launcher EXE files include the GestArt icon.
- Fixes cover Proko anonymous sessions, pagination for full packs, and updating
  the saved file index on Windows.

Source code is in `ultimate`; the `old` branch and release 0.9.8 are preserved.
For instructions, see the [English README](https://github.com/LeonWGal/GestArt/blob/ultimate/README.md)
or the [portable guide in Russian](https://github.com/LeonWGal/GestArt/blob/ultimate/docs/portable.md).

## Русский

GestArt Ultimate 1.8.0 — версия программы для практики рисунка на C++20 и Qt Quick.
Она продолжает GestArt 0.9.8 и добавляет библиотеку коллекций, онлайн-источники
и инструменты для отслеживания практики.

### Отличия от GestArt 0.9.8

Таймер, перерывы, сетка, эффекты и работа со своими изображениями остаются
основой программы. Основные изменения Ultimate:

| Возможность | GestArt 0.9.8 | GestArt Ultimate 1.8.0 |
| --- | --- | --- |
| Основа приложения | Python и PyQt6 | C++20 и Qt Quick |
| Источники референсов | Локальные папки | Локальные папки и скачивание с Quickposes, Proko, Pose Library и SketchDaily |
| Организация | Просмотр папок и статистика по папкам | Библиотека коллекций, превью и сохранённые сеты для отдельных сессий |
| Сайты | Референсы добавляются из папок | Встроенный браузер, выбор сетов, полные паки и продолжение загрузок |
| Практика | История просмотров и статистика папок | История сессий, календарь практики и цели на день |

В версии 0.9.8 заявлены 100 языков; интерфейс Ultimate содержит 22 языковых
каталога. Часть переводов Ultimate выполнена машинно, отсутствующие строки
отображаются на английском.

### Скачивание

Для Windows 10 (1809+) / Windows 11 x64 выберите один файл:

- **Portable EXE** — программа и библиотеки в одном файле. Откройте EXE;
  при первом запуске он распакует библиотеки в локальный кэш.
  Требуется .NET Framework 4.8 или новее.
- **Portable ZIP** — распакуйте целиком и запустите `Start.cmd`.
  .NET Framework не нужен.
- **SHA256SUMS.txt** — контрольные суммы EXE и ZIP.

EXE хранит данные в `GestArtUltimate-data` рядом с собой, ZIP — в папке `data`.
Python и Qt SDK для запуска не нужны.

### Изменения в 1.8.0

- Скачивание разделено на выбор сайта, сетов и объёма с одной основной кнопкой.
- Добавлена вкладка **Сохранённые сеты**, которая остаётся после перезапуска.
  Перед запуском можно настроить таймер; другие коллекции не включаются.
- **Скачать весь пак** работает с Quickposes, Proko и Pose Library;
  SketchDaily сохраняет заданное количество случайных референсов.
- Появились превью, поиск, постоянный прогресс и продолжение частичных загрузок.
- Панель браузера при скрытии освобождает место изображению.
- Windows EXE приложения и portable-загрузчика содержат иконку GestArt.
- Исправлены анонимная сессия Proko, постраничная загрузка полных паков
  и обновление индекса файлов в Windows.

Исходники находятся в ветке `ultimate`; ветка `old` и релиз 0.9.8 сохранены.
Инструкции: [README на русском](https://github.com/LeonWGal/GestArt/blob/ultimate/README.ru.md),
[portable-версия](https://github.com/LeonWGal/GestArt/blob/ultimate/docs/portable.md).
