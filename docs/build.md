# Соберите GestArt Ultimate

Эта инструкция предназначена для разработчиков C++20. Она описывает сборку
приложения, тестов и portable-артефактов. Для запуска готовой программы
используйте [инструкцию portable](portable.md).

Сначала скачайте исходники [ветки ultimate](https://github.com/LeonWGal/GestArt/tree/ultimate).
Команды сборки выполняются в их корне, а не в распакованной portable-сборке.

## Подготовьте инструменты

Для Windows нужны Visual Studio 2022/2026 с компонентами C++ и Windows SDK,
CMake 3.24+, PowerShell 7 и Qt 6.8.3. Для установки Qt нужен Python;
готовое приложение от Python не зависит. Для portable EXE нужен компилятор
C# из .NET Framework 4.8 и штатные библиотеки сжатия Windows.

Приложение использует Qt Core, Gui, Qml, Quick, QuickControls2, Concurrent,
Sql, Svg, Multimedia, ShaderTools, Network, WebEngineQuick и Test.

В корне проекта установите SDK и дополнительные модули:

```powershell
python -m pip install aqtinstall==3.3.0
python -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 `
  -O .deps/Qt -m qtshadertools qtmultimedia qtimageformats `
  qtwebengine qtwebchannel qtpositioning qtserialport
```

Для внешнего SDK передайте `-QtRoot QT_SDK_PATH` в скрипт сборки.
Замените `QT_SDK_PATH` путём к каталогу Qt с папками `bin`, `lib` и `qml`.

## Соберите приложение и тесты

В PowerShell 7, в корне проекта выполните:

```powershell
./tools/build-codecs.ps1 -Generator 'Visual Studio 17 2022'
./tools/build.ps1 -Generator 'Visual Studio 17 2022' -Package
```

Для Visual Studio 2026 замените генератор на `Visual Studio 18 2026`.
Скрипт кодеков собирает libde265 1.1.3 и libheif 1.23.6; включён HEVC-декодер.
Скрипт приложения конфигурирует CMake, собирает Release, запускает CTest
и размещает программу с библиотеками в `release`.

Успешная проверка заканчивается `100% tests passed`. Откройте
`release/Start.cmd` и убедитесь, что появляется главная страница.

Если проект перемещён, используйте другой каталог сборки:

```powershell
./tools/build.ps1 -BuildDirectory build-clean -Package
```

## Соберите portable EXE и ZIP

После успешной сборки приложения выполните:

```powershell
./tools/package-portable.ps1
```

В `dist` появляются:

- `GestArt-Ultimate-1.8.0-windows-x64-portable.exe` — загрузочный EXE с ZIP
  внутри, проверкой SHA-256 и запуском Qt-приложения.
- `GestArt-Ultimate-1.8.0-windows-x64-portable.zip` — программа с библиотеками,
  `Start.cmd`, лицензиями, документацией и `portable.flag`.
- `SHA256SUMS.txt` — контрольные суммы EXE и ZIP.

Для другого каталога подготовленной сборки или артефактов передайте пути:

```powershell
./tools/package-portable.ps1 `
  -ReleaseDirectory RELEASE_PATH -OutputDirectory OUTPUT_PATH
```

Замените `RELEASE_PATH` на каталог автономной сборки, `OUTPUT_PATH` — на
пустой каталог результатов. Скрипт не перезаписывает существующий ZIP.
Он отказывается упаковывать найденные пользовательские профили и секреты.
Не запускайте приложение с данными внутри `release` перед упаковкой.

Исходник загрузочного EXE находится в `tools/portable/PortableLauncher.cs`.
Его встроенный архив проверяется перед извлечением; аргументы передаются
приложению, а код завершения возвращается вызывающему процессу.

## Проверьте изменение приложения

Для независимой проверки сессии используйте безопасную тестовую папку:

```powershell
./release/bin/GestArtUltimate.exe --source REFERENCES_PATH `
  --data-dir ./test-data/smoke --smoke-test
```

Замените `REFERENCES_PATH` на папку тестовых изображений. Код завершения 0
означает успех. QA-команды изменяют только указанный профиль; не передавайте
им свой рабочий профиль.

Дополнительные сценарии запускаются так:

```powershell
./release/bin/GestArtUltimate.exe --source REFERENCES_PATH `
  --data-dir ./test-data/ui --ui-test
./release/bin/GestArtUltimate.exe --web-test quickposes `
  --web-focus-test --data-dir ./test-data/web
./release/bin/GestArtUltimate.exe --online-test proko `
  --online-via-queue --online-count 0 --data-dir ./test-data/download
./release/bin/GestArtUltimate.exe --saved-flow-test `
  --data-dir ./test-data/download
```

`--online-count 0` проверяет полный пак. Онлайн-сценарии требуют интернета.
`--saved-flow-test` требует уже скачанного сета и проверяет настройку
и запуск именно этого сета. `--web-focus-test` проверяет освобождение
верхней области при уходе курсора.

Для снимка интерфейса используйте:

```powershell
./release/bin/GestArtUltimate.exe --data-dir ./test-data/capture `
  --capture ./downloader.png --capture-page downloader --size 360x440
```

Другие страницы захвата включают `home`, `studio`, `library`, `history`,
`online`, `saved-sets`, `saved-setup` и `settings`; полный список параметров
доступен через `--help`. Панели сессии проверяются в `--ui-test`, а не
физическим перемещением мыши.

## Соберите на Linux или macOS

Установите C++20-компилятор, CMake, Ninja, PowerShell 7 и соответствующий
Qt SDK с перечисленными модулями. Затем выполните:

```powershell
./tools/build-codecs.ps1 -Generator Ninja
./tools/build.ps1 -QtRoot QT_SDK_PATH -Generator Ninja -Package
```

Замените `QT_SDK_PATH` путём к SDK своей платформы. Для Bash доступен
`tools/build.sh` с переменной `QT_ROOT`. Готовые portable EXE предназначены
для Windows; Linux/macOS собираются из исходников. Локальный протокол
выпуска подтверждает Windows-сборку; успешную работу на других системах
проверяйте на соответствующих системах.

## Проверьте результат

Убедитесь, что CTest проходит, программа запускается без Qt SDK в `PATH`,
а portable-артефакты содержат библиотеки и лицензии. Смотрите
[протокол выпуска](verification.md) и [правила внесения изменений](../CONTRIBUTING.md).
