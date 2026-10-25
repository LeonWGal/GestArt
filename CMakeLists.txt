cmake_minimum_required(VERSION 3.24)
project(GestArtUltimate VERSION 1.8.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
include(CTest)
add_library(gestart_domain src/domain/session.cpp)
target_include_directories(gestart_domain PUBLIC src)
if(MSVC)
  add_compile_options(/utf-8 /W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Concurrent Sql Svg Multimedia ShaderTools Network WebEngineQuick Test)
find_package(libheif CONFIG QUIET)
qt_standard_project_setup(REQUIRES 6.8)
qt_add_executable(GestArtUltimate WIN32 MACOSX_BUNDLE
  src/main.cpp src/media/download_queue.cpp src/media/download_queue.h src/media/online_sources.cpp src/media/online_sources.h src/app/controller.cpp src/app/controller.h
  src/qa/polish_checks.cpp src/qa/polish_checks.h
  src/media/catalog.cpp src/media/catalog.h
  src/media/collections.cpp src/media/collections.h
  src/media/image_provider.cpp src/media/image_provider.h
  src/storage/store.cpp src/storage/store.h
  src/storage/practice_stats.cpp src/storage/practice_stats.h
  src/ui/localization.cpp src/ui/localization.h src/ui/themes.cpp src/ui/themes.h)
if(WIN32)
  target_sources(GestArtUltimate PRIVATE assets/version.rc)
  target_include_directories(GestArtUltimate PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/assets")
  target_link_libraries(GestArtUltimate PRIVATE user32)
endif()
target_link_libraries(GestArtUltimate PRIVATE gestart_domain
  Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2
  Qt6::Concurrent Qt6::Sql Qt6::Svg Qt6::Multimedia Qt6::ShaderTools Qt6::Network Qt6::WebEngineQuick Qt6::Test)
if(TARGET heif)
  target_sources(GestArtUltimate PRIVATE src/media/heif_decoder.cpp)
  target_compile_definitions(GestArtUltimate PRIVATE GESTART_HAS_HEIF)
  target_link_libraries(GestArtUltimate PRIVATE heif)
  if(WIN32)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/.deps/codecs/bin/heif.dll" "${CMAKE_CURRENT_SOURCE_DIR}/.deps/codecs/bin/libde265.dll" DESTINATION bin OPTIONAL)
  endif()
endif()
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(GestArtUltimate URI GestArt VERSION 1.0
  QML_FILES qml/OnlineSites.qml qml/WebStudio.qml qml/OnlineSourcesSheet.qml qml/Main.qml qml/MediaPreview.qml qml/Theme.qml qml/LobeButton.qml qml/LobeSwitch.qml qml/IconButton.qml
  qml/ArtIcon.qml qml/WindowGrip.qml qml/Sidebar.qml qml/Studio.qml qml/Library.qml qml/CollectionList.qml qml/CollectionsSheet.qml
  qml/SettingsSheet.qml qml/ThemePicker.qml qml/LobeMenu.qml qml/LobeMenuItem.qml qml/LobeComboBox.qml qml/LobeSegmented.qml qml/LobeSearch.qml qml/LobeNotice.qml qml/LobeEmpty.qml qml/LobeCheckBox.qml qml/History.qml qml/ValueField.qml
  qml/Home.qml qml/Practice.qml qml/PracticeCalendar.qml qml/MetricCard.qml qml/GoalCard.qml qml/WeekStrip.qml qml/CalendarGrid.qml qml/PracticeChart.qml qml/GlassPanel.qml qml/ImageGrid.qml qml/SessionResult.qml qml/SessionToolbar.qml qml/SessionPopover.qml qml/SessionSetup.qml qml/ActivityStrip.qml)
file(GLOB_RECURSE ART_ASSETS CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "assets/*")
list(FILTER ART_ASSETS EXCLUDE REGEX "^assets/screenshots/")
qt_add_resources(GestArtUltimate "art_assets" PREFIX "/" FILES ${ART_ASSETS})
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/grayscale.frag.qsb"
  COMMAND Qt6::qsb --glsl "100es,120,150" --hlsl 50 --msl 12
    -o "${CMAKE_CURRENT_BINARY_DIR}/grayscale.frag.qsb" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/grayscale.frag"
  DEPENDS shaders/grayscale.frag Qt6::qsb VERBATIM)
qt_add_resources(GestArtUltimate "art_shaders" PREFIX "/shaders"
  BASE "${CMAKE_CURRENT_BINARY_DIR}" FILES "${CMAKE_CURRENT_BINARY_DIR}/grayscale.frag.qsb")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/glass.frag.qsb"
  COMMAND Qt6::qsb --glsl "100es,120,150" --hlsl 50 --msl 12
    -o "${CMAKE_CURRENT_BINARY_DIR}/glass.frag.qsb" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/glass.frag"
  DEPENDS shaders/glass.frag Qt6::qsb VERBATIM)
qt_add_resources(GestArtUltimate "glass_shaders" PREFIX "/shaders"
  BASE "${CMAKE_CURRENT_BINARY_DIR}" FILES "${CMAKE_CURRENT_BINARY_DIR}/glass.frag.qsb")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/blur.frag.qsb"
  COMMAND Qt6::qsb --glsl "100es,120,150" --hlsl 50 --msl 12
    -o "${CMAKE_CURRENT_BINARY_DIR}/blur.frag.qsb" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/blur.frag"
  DEPENDS shaders/blur.frag Qt6::qsb VERBATIM)
qt_add_resources(GestArtUltimate "blur_shaders" PREFIX "/shaders"
  BASE "${CMAKE_CURRENT_BINARY_DIR}" FILES "${CMAKE_CURRENT_BINARY_DIR}/blur.frag.qsb")
set_target_properties(GestArtUltimate PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME "GestArt Ultimate"
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.gestart.ultimate"
  MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
install(TARGETS GestArtUltimate BUNDLE DESTINATION . RUNTIME DESTINATION bin)
if(MSVC)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION bin)
  include(InstallRequiredSystemLibraries)
endif()
if(TARGET Qt6::QOffscreenIntegrationPlugin)
  install(FILES "$<TARGET_FILE:Qt6::QOffscreenIntegrationPlugin>" DESTINATION plugins/platforms)
endif()
if(WIN32)
  # Qt 6.8's convenience macro emits an unquoted executable path; the user-selected
  # project directory contains spaces. Keep the supported deployment API and quote it.
  qt_generate_deploy_script(TARGET GestArtUltimate OUTPUT_SCRIPT deploy_script CONTENT
    "qt6_deploy_qml_imports(TARGET GestArtUltimate PLUGINS_FOUND plugins_found)
qt6_deploy_runtime_dependencies(EXECUTABLE \"$<TARGET_FILE:GestArtUltimate>\"
  ADDITIONAL_MODULES \${plugins_found} GENERATE_QT_CONF
  DEPLOY_TOOL_OPTIONS --qmldir \"${CMAKE_CURRENT_SOURCE_DIR}/qml\")")
elseif(UNIX AND NOT APPLE)
  qt_generate_deploy_script(TARGET GestArtUltimate OUTPUT_SCRIPT deploy_script CONTENT
    "qt6_deploy_qml_imports(TARGET GestArtUltimate PLUGINS_FOUND plugins_found)
file(GLOB_RECURSE qml_runtime_modules \"\${QT_DEPLOY_PREFIX}/qml/*${CMAKE_SHARED_MODULE_SUFFIX}\")
qt6_deploy_runtime_dependencies(EXECUTABLE \"$<TARGET_FILE:GestArtUltimate>\"
  ADDITIONAL_MODULES \${qml_runtime_modules} GENERATE_QT_CONF)")
else()
  qt_generate_deploy_qml_app_script(TARGET GestArtUltimate OUTPUT_SCRIPT deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
endif()
install(SCRIPT ${deploy_script})
install(FILES README.md README.ru.md LICENSE THIRD_PARTY_NOTICES.md CHANGELOG.md CONTRIBUTING.md DESTINATION .)
install(DIRECTORY assets/screenshots DESTINATION assets)
if(WIN32)
  install(FILES Start.cmd DESTINATION .)
endif()
install(DIRECTORY licenses docs DESTINATION .)
install(DIRECTORY assets/fonts DESTINATION lib)
if(UNIX AND NOT APPLE)
  install(FILES assets/com.gestart.ultimate.desktop DESTINATION share/applications)
  install(FILES assets/app-icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME com.gestart.ultimate.svg)
endif()
if(BUILD_TESTING)
  add_executable(session_tests tests/session_tests.cpp)
  target_link_libraries(session_tests PRIVATE gestart_domain)
  add_test(NAME session_rules COMMAND session_tests)
  qt_add_executable(integration_tests tests/integration_tests.cpp
    src/media/catalog.cpp src/media/catalog.h src/media/collections.cpp src/media/collections.h src/media/image_provider.cpp src/media/image_provider.h
    src/storage/store.cpp src/storage/store.h src/storage/practice_stats.cpp src/storage/practice_stats.h src/ui/themes.cpp src/ui/themes.h src/ui/localization.cpp src/ui/localization.h)
  qt_add_resources(integration_tests "test_themes" PREFIX "/" FILES assets/themes.json assets/icons/home.svg)
  file(GLOB LOCALE_TEST_ASSETS CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "assets/locales-v2/*.json")
  qt_add_resources(integration_tests "test_localization" PREFIX "/" FILES
    ${LOCALE_TEST_ASSETS} assets/languages.json tools/localization/targets.json)
  target_link_libraries(integration_tests PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Concurrent Qt6::Sql Qt6::Svg Qt6::Test)
  target_include_directories(integration_tests PRIVATE src)
  target_compile_definitions(integration_tests PRIVATE GESTART_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")
  if(TARGET heif)
    target_sources(integration_tests PRIVATE src/media/heif_decoder.cpp)
    target_compile_definitions(integration_tests PRIVATE GESTART_HAS_HEIF)
    target_link_libraries(integration_tests PRIVATE heif)
  endif()
  add_test(NAME media_and_storage COMMAND integration_tests)
  set_tests_properties(media_and_storage PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

if(BUILD_TESTING)
  qt_add_executable(online_tests tests/online_tests.cpp tests/online_tests.h src/media/download_queue.cpp src/media/download_queue.h src/media/online_sources.cpp src/media/online_sources.h)
  target_include_directories(online_tests PRIVATE src)
  target_link_libraries(online_tests PRIVATE Qt6::Core Qt6::Gui Qt6::Network Qt6::Test)
  add_test(NAME online_providers COMMAND online_tests)
endif()
