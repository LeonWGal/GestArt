import sys
import os
import random
import logging
import json
from send2trash import send2trash

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QFileDialog, QDialog, QCheckBox, QSpinBox,
    QLineEdit, QComboBox, QMessageBox, QStatusBar, QGridLayout, QFrame,
    QSlider, QSizePolicy, QButtonGroup, QProgressBar, QSplitter, QScrollArea,
    QStackedLayout, QToolTip
)
from PyQt6.QtGui import (
    QPixmap, QImage, QTransform, QPainter, QShortcut, QKeySequence, QPen, QColor, 
    QLinearGradient, QIcon, QPalette, QDesktopServices, QPainterPath
)
from PyQt6.QtCore import (
    QTimer, Qt, pyqtSignal, QThread, QPointF, QSize, pyqtProperty,
    QPropertyAnimation, QEasingCurve, QEvent, QRect, QMargins, QRectF, QUrl
)
from PyQt6.QtSvg import QSvgRenderer
from PyQt6.QtMultimedia import QMediaPlayer, QAudioOutput
from PyQt6.QtWidgets import QStyle  # Добавляем импорт QStyle

from locales.translations import translation_manager, tr
from themes.thememanager import theme_manager


def get_resource_path(relative_path):
    """Получает абсолютный путь к ресурсу, работает как в dev, так и в PyInstaller"""
    try:
        # PyInstaller создает временную папку и сохраняет путь в _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        # В режиме разработки используем текущую папку
        base_path = os.path.abspath(".")
    
    return os.path.join(base_path, relative_path)


def get_font_family_for_language(language_code):
    """Возвращает подходящее семейство шрифтов для языка"""
    # Карта языков к специальным шрифтам
    language_fonts = {
        # CJK языки
        'ko': '"Malgun Gothic", "맑은 고딕", "Noto Sans CJK KR", "Apple SD Gothic Neo", "Segoe UI", sans-serif',  # Корейский
        'ja': '"Yu Gothic UI", "Meiryo UI", "Noto Sans CJK JP", "Hiragino Sans", "Segoe UI", sans-serif',  # Японский
        'zh-CN': '"Microsoft YaHei UI", "SimSun", "Noto Sans CJK SC", "PingFang SC", "Segoe UI", sans-serif',  # Китайский упрощенный
        'zh-TW': '"Microsoft JhengHei UI", "PMingLiU", "Noto Sans CJK TC", "PingFang TC", "Segoe UI", sans-serif',  # Китайский традиционный
        
        # Арабские языки
        'ar': '"Segoe UI", "Tahoma", "Arabic Typesetting", sans-serif',  # Арабский
        'fa': '"Segoe UI", "Tahoma", "Iranian Sans", sans-serif',  # Персидский
        'ur': '"Segoe UI", "Tahoma", "Urdu Typesetting", sans-serif',  # Урду
        
        # Индийские языки
        'hi': '"Segoe UI", "Nirmala UI", "Mangal", sans-serif',  # Хинди
        'bn': '"Segoe UI", "Nirmala UI", "Vrinda", sans-serif',  # Бенгальский
        'gu': '"Segoe UI", "Nirmala UI", "Shruti", sans-serif',  # Гуджарати
        'kn': '"Segoe UI", "Nirmala UI", "Tunga", sans-serif',  # Каннада
        'ml': '"Segoe UI", "Nirmala UI", "Kartika", sans-serif',  # Малаялам
        'mr': '"Segoe UI", "Nirmala UI", "Mangal", sans-serif',  # Маратхи
        'pa': '"Segoe UI", "Nirmala UI", "Raavi", sans-serif',  # Панджаби
        'ta': '"Segoe UI", "Nirmala UI", "Latha", sans-serif',  # Тамильский
        'te': '"Segoe UI", "Nirmala UI", "Gautami", sans-serif',  # Телугу
        'si': '"Segoe UI", "Nirmala UI", "Iskoola Pota", sans-serif',  # Сингальский
        
        # Тайские языки
        'th': '"Segoe UI", "Leelawadee UI", "Tahoma", sans-serif',  # Тайский
        'lo': '"Segoe UI", "Lao UI", "DokChampa", sans-serif',  # Лаосский
        'km': '"Segoe UI", "Leelawadee UI", "Khmer UI", sans-serif',  # Кхмерский
        'my': '"Segoe UI", "Myanmar Text", "Padauk", sans-serif',  # Бирманский
        
        # Эфиопские языки
        'am': '"Segoe UI", "Ebrima", "Nyala", sans-serif',  # Амхарский
        
        # Африканские языки
        'ff': '"Segoe UI", "Ebrima", sans-serif',  # Фула
        'ha': '"Segoe UI", "Ebrima", sans-serif',  # Хауса
        'ig': '"Segoe UI", "Ebrima", sans-serif',  # Игбо
        'yo': '"Segoe UI", "Ebrima", sans-serif',  # Йоруба
    }
    
    return language_fonts.get(language_code, '"Segoe UI", sans-serif')


def get_font_size_for_language(language_code):
    """Возвращает адаптивные размеры шрифтов для языков с длинными словами"""
    # Языки с очень длинными словами требуют меньшего размера шрифта
    compact_languages = {
        # Индийские языки с длинными словами
        'ml': {'base': 8, 'label': 11, 'header': 12, 'tooltip': 10},  # Малаялам (уменьшено)
        'ta': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Тамильский
        'te': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Телугу
        'kn': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Каннада
        'bn': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Бенгальский
        'hi': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Хинди
        'gu': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Гуджарати
        'pa': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Панджаби
        'or': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Ория
        'as': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Ассамский
        'ne': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Непальский
        'si': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Сингальский
        
        # CJK языки с особыми требованиями к размеру шрифта
        'ko': {'base': 11, 'label': 14, 'header': 15, 'tooltip': 12},  # Корейский - увеличенный размер для лучшей читаемости
        'ja': {'base': 11, 'label': 14, 'header': 15, 'tooltip': 12},  # Японский - увеличенный размер для лучшей читаемости
        'zh-CN': {'base': 11, 'label': 14, 'header': 15, 'tooltip': 12},  # Китайский упрощенный
        'zh-TW': {'base': 11, 'label': 14, 'header': 15, 'tooltip': 12},  # Китайский традиционный
        
        # Финно-угорские языки с агглютинацией
        'fi': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Финский
        'et': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Эстонский
        'hu': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Венгерский
        'myv': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11}, # Эрзянский
        'mdf': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11}, # Мокшанский
        
        # Тюркские языки
        'tr': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Турецкий
        'az': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Азербайджанский
        'kk': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Казахский
        'ky': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Киргизский
        'uz': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Узбекский
        'tt': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Татарский
        'ba': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Башкирский
        'cv': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Чувашский
        'sah': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11}, # Якутский
        'bua': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11}, # Бурятский
        
        # Германские языки с длинными составными словами
        'de': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Немецкий
        'nl': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Голландский
        'da': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Датский
        'no': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Норвежский
        'sv': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Шведский
        'is': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Исландский
        
        # Другие языки с длинными словами
        'th': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Тайский
        'km': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Кхмерский
        'my': {'base': 8, 'label': 11, 'header': 12, 'tooltip': 10},  # Бирманский (уменьшено)
        'am': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Амхарский
        'ti': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Тигринья
        'om': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Оромо
        'so': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Сомали
        'sw': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Суахили
        'zu': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Зулу
        'xh': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Коса
        'af': {'base': 9, 'label': 12, 'header': 13, 'tooltip': 11},  # Африкаанс
    }
    
    # Размеры по умолчанию для остальных языков
    default_sizes = {'base': 10, 'label': 14, 'header': 14, 'tooltip': 12}
    
    return compact_languages.get(language_code, default_sizes)


# Кэш для иконок
_icon_cache = {}

# Функция для создания иконок в соответствии с темой
def create_themed_icon(icon_path):
    # Проверяем кэш
    cache_key = f"{icon_path}_{theme_manager.get_current_theme()}"
    if cache_key in _icon_cache:
        return _icon_cache[cache_key]
    
    # Получаем правильный путь к ресурсу
    full_path = get_resource_path(icon_path)
    pixmap = QPixmap(full_path)
    # Проверяем, загружено ли изображение
    if pixmap.isNull():
        logging.warning(f"Failed to load icon: {icon_path}")
        return QIcon()  # Возвращаем пустую иконку
    
    # Создаем пустой QPixmap того же размера с прозрачным фоном
    themed_pixmap = QPixmap(pixmap.size())
    themed_pixmap.fill(Qt.GlobalColor.transparent)
    
    # Создаем QPainter для рисования на новом QPixmap
    painter = QPainter(themed_pixmap)
    # Устанавливаем композиционный режим для замены цветов
    painter.setCompositionMode(QPainter.CompositionMode.CompositionMode_Source)
    # Рисуем оригинальную иконку
    painter.drawPixmap(0, 0, pixmap)
    # Меняем композиционный режим для наложения цвета
    painter.setCompositionMode(QPainter.CompositionMode.CompositionMode_SourceIn)
    
    # Получаем цвет текста из текущей темы
    colors = theme_manager.get_theme_colors()
    text_color = colors['text']
    
    # Преобразуем text_color из разных форматов в QColor
    icon_color = QColor()
    if text_color.startswith('#'):
        # Если цвет в формате HEX (#ffffff)
        icon_color.setNamedColor(text_color)
    elif text_color.startswith('rgba'):
        # Если цвет в формате rgba
        try:
            # Извлекаем числа из строки rgba(r, g, b, a)
            parts = text_color.strip('rgba()').split(',')
            r = int(parts[0].strip())
            g = int(parts[1].strip())
            b = int(parts[2].strip())
            a = int(float(parts[3].strip()) * 255 / 250)  # Преобразуем alpha из 0-250 в 0-255
            icon_color = QColor(r, g, b, a)
        except Exception as e:
            logging.warning(f"Color conversion error: {e}")
            icon_color = QColor(255, 255, 255)  # Белый по умолчанию
    else:
        # Пробуем преобразовать как именованный цвет
        icon_color.setNamedColor(text_color)
        if not icon_color.isValid():
            # Если не удалось, используем белый цвет по умолчанию
            icon_color = QColor(255, 255, 255)
    
    # Заливаем иконку полученным цветом
    painter.fillRect(themed_pixmap.rect(), icon_color)
    painter.end()
    
    # Создаем иконку и сохраняем в кэш
    icon = QIcon(themed_pixmap)
    _icon_cache[cache_key] = icon
    return icon

# Logger configuration
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s - %(levelname)s - %(message)s')

class ConfigManager:
    def __init__(self):
        self.config_dir = os.path.join(os.path.expanduser("~"), ".gestart")
        self.settings_file = os.path.join(self.config_dir, "settings.json")
        self.history_file = os.path.join(self.config_dir, "history.json")
        self.folder_stats_file = os.path.join(self.config_dir, "folder_stats.json")
        self.ensure_config_dir()
        
    def ensure_config_dir(self):
        """Creates configuration directory if it doesn't exist"""
        if not os.path.exists(self.config_dir):
            os.makedirs(self.config_dir)
            
    def save_settings(self, settings):
        """Saves settings to JSON file"""
        with open(self.settings_file, 'w', encoding='utf-8') as f:
                json.dump(settings, f, ensure_ascii=False, indent=4)
            
    def load_settings(self):
        """Загружает настройки из файла"""
        default_settings = {
            "folder": "",
            "display_time": 30,
            "num_images": 10,
            "save_history": True,
            "use_break": False,
            "break_duration": 300,  # 5 минут в секундах
            "timer_position": "Center",
            "show_timer": True,
            "unlimited_time": False,
            "unlimited_images": False,
            "unlimited_break": False,
            "language": "en",
            "always_on_top": False,
            "preview_mode": False,
            "confirm_delete": True,
            "grid_h_lines": 2,
            "grid_v_lines": 2,
            "timer_volume": 50,
            "theme": "dark"
        }
        
        if os.path.exists(self.settings_file):
            try:
                with open(self.settings_file, 'r', encoding='utf-8') as f:
                    saved_settings = json.load(f)
                    
                    # Compatibility: Check for old 'enable_breaks' key first
                    if "enable_breaks" in saved_settings:
                        saved_settings["use_break"] = saved_settings["enable_breaks"]
                        # Optionally remove the old key to clean up the config over time
                        # del saved_settings["enable_breaks"] 
                    
                    # Update default settings with saved values
                    for key in default_settings:
                        if key in saved_settings:
                            default_settings[key] = saved_settings[key]
            except Exception as e:
                logging.error(f"Error loading settings file {self.settings_file}: {e}")
                # Fallback to default settings if loading fails
            
        return default_settings
        
    def save_history(self, history):
        """Saves viewed images history to JSON file"""
        with open(self.history_file, 'w', encoding='utf-8') as f:
            json.dump(list(history), f, ensure_ascii=False, indent=4)
            
    def load_history(self):
        """Loads viewed images history from JSON file"""
        if os.path.exists(self.history_file):
            with open(self.history_file, 'r', encoding='utf-8') as f:
                return list(json.load(f))
        return []

    def save_folder_stats(self, folder_stats):
        """Saves viewing statistics for each folder"""
        with open(self.folder_stats_file, 'w', encoding='utf-8') as f:
            json.dump(folder_stats, f, ensure_ascii=False, indent=4)

    def load_folder_stats(self):
        """Loads viewing statistics for each folder"""
        if os.path.exists(self.folder_stats_file):
            with open(self.folder_stats_file, 'r', encoding='utf-8') as f:
                return json.load(f)
        return {}

# Асинхронный сканер папки для повышения производительности
class FolderScannerThread(QThread):
    scanned = pyqtSignal(list)
    error = pyqtSignal(str)
    progress = pyqtSignal(int, int)

    def __init__(self, folder, supported_extensions, parent=None):
        super().__init__(parent)
        self.folder = folder
        self.supported_extensions = supported_extensions
        self.is_running = True

    def stop(self):
        self.is_running = False

    def count_total_files(self, folder):
        total = 0
        try:
            with os.scandir(folder) as it:
                for entry in it:
                    if not self.is_running:
                        return 0
                    if entry.is_file():
                        total += 1
                    elif entry.is_dir():
                        total += self.count_total_files(entry.path)
        except Exception:
            pass
        return total

    def scan_folder(self, folder, image_files, processed_files, total_files):
        try:
            with os.scandir(folder) as it:
                for entry in it:
                    if not self.is_running:
                        return
                        
                    try:
                        if entry.is_file():
                            if entry.name.lower().endswith(self.supported_extensions):
                                full_path = os.path.join(folder, entry.name)
                                if os.access(full_path, os.R_OK):
                                    image_files.append(full_path)
                        elif entry.is_dir():
                            self.scan_folder(entry.path, image_files, processed_files, total_files)
                    except Exception as e:
                        logging.warning(f"Error processing {entry.name}: {e}")
                        continue
                        
                    processed_files[0] += 1
                    if processed_files[0] % 10 == 0:
                        self.progress.emit(processed_files[0], total_files)
                        
        except Exception as e:
            logging.warning(f"Error scanning folder {folder}: {e}")

    def run(self):
        image_files = []
        try:
            if not os.path.exists(self.folder):
                self.error.emit(tr("Folder does not exist") + f": {self.folder}")
                return
                
            if not os.access(self.folder, os.R_OK):
                self.error.emit(tr("No access to folder") + f": {self.folder}")
                return

            # Count total number of files for progress tracking
            total_files = self.count_total_files(self.folder)
            processed_files = [0]  # Using list for pass by reference
            
            # Start recursive scanning
            self.scan_folder(self.folder, image_files, processed_files, total_files)
            
            if self.is_running:
                if not image_files:
                    self.error.emit(tr("No images found in selected folder and subfolders"))
                else:
                    self.scanned.emit(image_files)
                    
        except Exception as e:
            self.error.emit(tr("Error") + f": {e}")
            logging.error(f"Error scanning folder: {e}")

# Settings window (interface 1)
class SettingsDialog(QDialog):
    settings_updated = pyqtSignal(dict)

    def __init__(self, parent=None, current_settings=None):
        super().__init__(parent)
        
        self.setWindowTitle(tr("Settings"))
        
        # Set correct window flags
        self.setWindowFlags(Qt.WindowType.Dialog | Qt.WindowType.WindowCloseButtonHint)
        
        # Enable drag & drop support for the entire window
        self.setAcceptDrops(True)
        
        # Save current settings
        self.current_settings = current_settings or {}
        
        # Set default values for new settings if they don't exist
        if "timer_volume" not in self.current_settings:
            self.current_settings["timer_volume"] = 50
        if "theme" not in self.current_settings:
            self.current_settings["theme"] = "dark"
        # Only default 'use_break'
        if "use_break" not in self.current_settings:
            # Compatibility check (load from old key if new one is missing)
            self.current_settings["use_break"] = self.current_settings.get("enable_breaks", False)
        
        # Переменные для таймера и звука
        self.is_paused = False
        self.timer_sound_playing = False
        self.countdown_timer = None
        
        # Флаг для переключения между интерфейсами настроек и информации
        self.show_info_interface = False
        
        # Set minimum window dimensions
        self.setMinimumWidth(680)  # Уменьшаем минимальную ширину
        self.setMinimumHeight(720)  # Уменьшаем минимальную высоту
        
        # Setup UI
        self.setup_ui()
        
        # Connect signals
        self.connect_signals()
        
        # Center window
        self.center_on_parent()
        
        # Load preview if folder is already selected
        if self.current_settings.get("folder"):
            self.folder_drop.set_folder(self.current_settings["folder"])
            
            # If preview mode is enabled, activate it
            if self.current_settings.get("preview_mode", False):
                self.folder_drop.preview_mode_btn.setChecked(True)
                self.folder_drop.on_mode_changed(self.folder_drop.preview_mode_btn)
                
            # Update folder label with the current folder's statistics
            self.update_folder_label(self.current_settings.get("folder"))
                
        # Update number of images in history
        self.update_history_count()
        
        # Применяем тему
        self.apply_theme()

    def update_history_count(self):
        """Updates the display of the number of images in history"""
        if hasattr(self.parent(), 'displayed_history'):
            count = len(self.parent().displayed_history)
            self.history_count_label.setText(f"{tr('Total in history')}: {count}")
        else:
            self.history_count_label.setText(f"{tr('Total in history')}: 0")

    def center_on_parent(self):
        """Centers the window relative to its parent window"""
        if self.parent():
            parent_geometry = self.parent().geometry()
            x = parent_geometry.center().x() - self.width() // 2
            y = parent_geometry.center().y() - self.height() // 2
            self.move(x, y)

    def resizeEvent(self, event):
        """Handles window resize event"""
        super().resizeEvent(event)
        # Don't fix the size so the system can properly manage the window size

    def moveEvent(self, event):
        """Handles window move event"""
        super().moveEvent(event)
        # Don't fix the size when moving

    def setup_ui(self):
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Dialog)
        self.setModal(True)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        
        # Main container with shadow and rounded corners
        self.container = QWidget(self)
        self.container.setObjectName("settingsContainer")
        
        # Main layout
        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(20, 20, 20, 20)
        
        # Container layout
        container_layout = QVBoxLayout(self.container)
        container_layout.setSpacing(6)  # Уменьшаем отступы между элементами
        container_layout.setContentsMargins(16, 16, 16, 16)  # Уменьшаем отступы контейнера

        # Header
        header_layout = QHBoxLayout()
        header_layout.setSpacing(8)
        
        # Заголовок (который будет меняться в зависимости от интерфейса)
        self.title_label = QLabel(tr("Settings"))
        self.title_label.setStyleSheet("font-size: 18px; font-weight: bold;")
        header_layout.addWidget(self.title_label)
        header_layout.addStretch()
        
        # Кнопка информации для переключения интерфейса
        self.info_button = QPushButton()
        self.info_button.setFixedSize(28, 28)
        self.info_button.setIcon(create_themed_icon("resources/info.png")) # Используем иконку
        self.info_button.setIconSize(QSize(16, 16)) # Устанавливаем размер иконки
        self.info_button.setStyleSheet("""
            QPushButton {
                background-color: transparent;
                border: none;
                padding: 0;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 10);
            }
        """)
        self.info_button.clicked.connect(self.toggle_info_interface)
        header_layout.addWidget(self.info_button)
        
        # Кнопка закрытия с иконкой exit.png
        self.close_button = QPushButton()
        self.close_button.setFixedSize(28, 28)
        self.close_button.setIcon(create_themed_icon("resources/exit.png"))
        self.close_button.setIconSize(QSize(16, 16))
        self.close_button.setStyleSheet("""
            QPushButton {
                background-color: transparent;
                border: none;
                padding: 0;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 10);
            }
        """)
        self.close_button.clicked.connect(self.reject)
        header_layout.addWidget(self.close_button)
        
        container_layout.addLayout(header_layout)

        # Separator after header
        container_layout.addWidget(self.create_separator())

        # Создаем два стека виджетов - для настроек и для информации
        self.stack_widget = QWidget()
        self.stack_layout = QStackedLayout(self.stack_widget)
        
        # Создаем основной контейнер настроек
        self.settings_widget = QWidget()
        settings_widget_layout = QVBoxLayout(self.settings_widget)
        settings_widget_layout.setContentsMargins(0, 0, 0, 0)
        settings_widget_layout.setSpacing(8)
        
        # Создаем информационный виджет
        self.info_widget = QWidget()
        info_widget_layout = QVBoxLayout(self.info_widget)
        info_widget_layout.setContentsMargins(0, 0, 0, 0)
        info_widget_layout.setSpacing(8)
        
        # Добавляем виджеты в стек
        self.stack_layout.addWidget(self.settings_widget)
        self.stack_layout.addWidget(self.info_widget)
        
        # Делаем фон информационного окна прозрачным
        # self.info_widget.setStyleSheet("background: transparent;")
        # info_scroll.setStyleSheet("background: transparent;")
        # info_content.setStyleSheet("background: transparent;")
        
        # Добавляем стек в основной контейнер
        container_layout.addWidget(self.stack_widget)
        
        # Настраиваем содержимое виджета настроек (существующий код с небольшими изменениями)
        self.setup_settings_ui(settings_widget_layout)
        
        # Настраиваем содержимое информационного виджета
        self.setup_info_ui(info_widget_layout)
        
        # Добавляем главный контейнер в основной макет
        main_layout.addWidget(self.container)

    def setup_settings_ui(self, layout):
        """Настраивает интерфейс вкладки настроек"""
        # Folder selection section - leave unchanged
        folder_section = QWidget()
        folder_layout = QVBoxLayout(folder_section)
        folder_layout.setContentsMargins(0, 0, 0, 0)
        folder_layout.setSpacing(6)

        # Current folder
        self.current_folder_label = QLabel()
        self.current_folder_label.setWordWrap(True)
        self.current_folder_label.setMinimumHeight(40)
        self.current_folder_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.current_folder_label.setText(tr("No folder selected"))
        folder_layout.addWidget(self.current_folder_label)

        # Folder drop area
        self.folder_drop = FolderDropArea(self)
        self.folder_drop.folder_dropped.connect(self.on_folder_dropped)
        folder_layout.addWidget(self.folder_drop)

        layout.addWidget(folder_section)

        # Main settings container with columns
        main_settings_container = QWidget()
        main_settings_layout = QHBoxLayout(main_settings_container)
        main_settings_layout.setContentsMargins(0, 6, 0, 6)  # Уменьшаем вертикальные отступы
        main_settings_layout.setSpacing(16)  # Уменьшаем расстояние между колонками

        # Create columns
        left_column = QWidget()
        right_column = QWidget()
        
        # Устанавливаем минимальную ширину колонок
        left_column.setMinimumWidth(310)  # Уменьшаем минимальную ширину
        right_column.setMinimumWidth(290)  # Уменьшаем минимальную ширину

        left_column_layout = QVBoxLayout(left_column)
        left_column_layout.setSpacing(6)  # Уменьшаем отступы между элементами
        left_column_layout.setContentsMargins(0, 0, 0, 0)

        right_column_layout = QVBoxLayout(right_column)
        right_column_layout.setSpacing(6)  # Уменьшаем отступы между элементами
        right_column_layout.setContentsMargins(0, 0, 0, 0)

        # Left column - session settings
        timing_layout = QVBoxLayout()
        timing_layout.setContentsMargins(0, 0, 0, 0)
        timing_layout.setSpacing(8)

        timing_label = QLabel(tr("Session settings"))
        timing_label.setProperty('isHeader', True)
        timing_label.original_text = "Session settings"
        left_column_layout.addWidget(timing_label)

        # Время показа
        self.time_adjuster = ValueAdjuster(
            tr("Display time (sec):"), 1, 900, 15,
            self.current_settings.get("display_time", 30)
        )
        self.time_adjuster.setUnlimited(self.current_settings.get("unlimited_time", False))
        left_column_layout.addWidget(self.time_adjuster)

        # Количество изображений
        self.num_adjuster = ValueAdjuster(
            tr("Number of images:"), 1, 900, 5,
            self.current_settings.get("num_images", 10)
        )
        self.num_adjuster.setUnlimited(self.current_settings.get("unlimited_images", False))
        left_column_layout.addWidget(self.num_adjuster)

        # Перерывы
        break_header = QWidget()
        break_header_layout = QHBoxLayout(break_header)
        break_header_layout.setContentsMargins(0, 8, 0, 0)
        break_header_layout.setSpacing(0)
        
        break_label = QLabel(tr("Break settings"))
        break_label.setProperty('isHeader', True)
        break_label.original_text = "Break settings"
        break_header_layout.addWidget(break_label)
        break_header_layout.addStretch()
        
        left_column_layout.addWidget(break_header)

        self.break_checkbox = QCheckBox(tr("Enable breaks"))
        # Read only 'use_break'
        self.break_checkbox.setChecked(self.current_settings.get("use_break", False))
        left_column_layout.addWidget(self.break_checkbox)

        # Изменяем компоновку для настроек перерыва, чтобы слайдер был нормальной ширины
        break_settings = QWidget()
        break_settings_layout = QVBoxLayout(break_settings)
        break_settings_layout.setContentsMargins(0, 0, 0, 0)  # Убираем отступ слева для выравнивания с другими слайдерами
        break_settings_layout.setSpacing(4)

        self.break_adjuster = ValueAdjuster(
            tr("Duration (min):"), 1, 60, 1,
            self.current_settings.get("break_duration", 300) // 60  # Конвертируем секунды в минуты
        )
        self.break_adjuster.setUnlimited(self.current_settings.get("unlimited_break", False))
        self.break_adjuster.unlimited_checkbox.hide()  # Скрываем чекбокс бесконечности для перерывов
        break_settings_layout.addWidget(self.break_adjuster)
        left_column_layout.addWidget(break_settings)
        
        # Добавляем слайдер громкости таймера
        timer_sound_header = QWidget()
        timer_sound_header_layout = QHBoxLayout(timer_sound_header)
        timer_sound_header_layout.setContentsMargins(0, 8, 0, 0)
        timer_sound_header_layout.setSpacing(0)
        
        # Сохраняем ссылку на метку как атрибут класса для обновления переводов
        self.timer_sound_label = QLabel(tr("Timer Sound"))
        self.timer_sound_label.setStyleSheet("font-weight: bold; font-size: 14px; margin-bottom: 6px;")
        self.timer_sound_label.setProperty('isHeader', True)  # Добавляем свойство для заголовка
        self.timer_sound_label.original_text = "Timer Sound"  # Сохраняем оригинальный ключ
        timer_sound_header_layout.addWidget(self.timer_sound_label)
        timer_sound_header_layout.addStretch()
        
        left_column_layout.addWidget(timer_sound_header)

        # Добавляем слайдер громкости таймера
        self.timer_volume_adjuster = ValueAdjuster(
            tr("Volume:"), 0, 100, 5,
            self.current_settings.get("timer_volume", 50)
        )
        self.timer_volume_adjuster.unlimited_checkbox.hide()  # Скрываем чекбокс бесконечности для громкости
        left_column_layout.addWidget(self.timer_volume_adjuster)
        
        # Добавляем растягивающийся элемент в конце левой колонки
        left_column_layout.addStretch()
        
        # Правая колонка - дополнительные настройки
        history_header = QWidget()
        history_header_layout = QHBoxLayout(history_header)
        history_header_layout.setContentsMargins(0, 0, 0, 0)
        history_header_layout.setSpacing(0)
        
        history_label = QLabel(tr("History"))
        history_label.setProperty('isHeader', True)
        history_label.original_text = "History"
        history_header_layout.addWidget(history_label)
        history_header_layout.addStretch()
        
        right_column_layout.addWidget(history_header)

        history_widget = QWidget()
        history_layout = QVBoxLayout(history_widget)
        history_layout.setContentsMargins(0, 0, 0, 0)
        history_layout.setSpacing(8)
        
        # Флажок сохранения истории
        self.history_checkbox = QCheckBox(tr("Save viewed images"))
        self.history_checkbox.setChecked(self.current_settings.get("save_history", True))
        history_layout.addWidget(self.history_checkbox)

        # Счетчик количества просмотренных изображений
        self.history_count_label = QLabel()
        self.history_count_label.setStyleSheet(theme_manager.get_theme_styles()["history_count"])
        history_layout.addWidget(self.history_count_label)

        # Кнопка очистки истории
        self.delete_history_button = QPushButton(tr("Clear history"))
        self.delete_history_button.clicked.connect(self.delete_history)
        history_layout.addWidget(self.delete_history_button)
        right_column_layout.addWidget(history_widget)

        # Флажок для подтверждения удаления
        self.confirm_delete_checkbox = QCheckBox(tr("Confirm file deletion"))
        self.confirm_delete_checkbox.setChecked(
            self.current_settings.get("confirm_delete", True)
        )
        right_column_layout.addWidget(self.confirm_delete_checkbox)

        # Позиция таймера
        timer_position_section = QWidget()
        timer_position_layout = QVBoxLayout(timer_position_section)
        timer_position_layout.setContentsMargins(0, 6, 0, 6)  # Стандартизируем отступы
        timer_position_layout.setSpacing(5)
        
        # Селектор позиции таймера
        self.timer_position = TimerPositionSelector()
        self.timer_position.setPosition(self.current_settings.get("timer_position", "center"))
        timer_position_layout.addWidget(self.timer_position)
        
        right_column_layout.addWidget(timer_position_section)

        # Настройки сетки (в правой колонке)
        grid_header = QWidget()
        grid_header_layout = QHBoxLayout(grid_header)
        grid_header_layout.setContentsMargins(0, 8, 0, 0)
        grid_header_layout.setSpacing(0)
        
        self.grid_settings_label = QLabel(tr("Grid Settings"))
        self.grid_settings_label.setProperty('isHeader', True)
        self.grid_settings_label.original_text = "Grid Settings"
        grid_header_layout.addWidget(self.grid_settings_label)
        grid_header_layout.addStretch()
        
        right_column_layout.addWidget(grid_header)

        # Контейнер для настроек сетки с отступом
        grid_content = QWidget()
        grid_content_layout = QVBoxLayout(grid_content)
        grid_content_layout.setContentsMargins(0, 0, 0, 0)
        grid_content_layout.setSpacing(8)
        
        # Создаем группу кнопок для горизонтальных/вертикальных линий
        def create_grid_buttons(group, default_value):
            colors = theme_manager.get_theme_colors()
            buttons_container = QWidget()
            buttons_layout = QHBoxLayout(buttons_container)
            buttons_layout.setContentsMargins(0, 0, 0, 0)
            buttons_layout.setSpacing(1)  # Минимальный отступ между кнопками
            
            for i in range(1, 4):
                btn = QPushButton(str(i))
                btn.setCheckable(True)
                btn.setFixedSize(40, 32)  # Увеличиваем ширину в 2,5 раза (с 16px до 40px)
                btn.setStyleSheet(f"""
                    QPushButton {{
                        background-color: {colors['background_secondary']};
                        color: {colors['text']};
                        border: 1px solid {colors['border']};
                        border-radius: 3px;
                        padding: 0px;
                        font-size: 14px;
                        min-width: 40px;
                        max-width: 40px;
                    }}
                    QPushButton:hover {{
                        background-color: {colors['background_hover']};
                        border: 1px solid {colors['border_hover']};
                    }}
                    QPushButton:checked {{
                        background-color: {colors['background_checked']};
                        border: 1px solid {colors['border_focus']};
                    }}
                """)
                if i == default_value:
                    btn.setChecked(True)
                group.addButton(btn, i)
                buttons_layout.addWidget(btn)
            
            return buttons_container
            
        # Горизонтальные линии
        h_lines_container = QWidget()
        h_lines_layout = QHBoxLayout(h_lines_container)
        h_lines_layout.setContentsMargins(0, 0, 0, 0)
        h_lines_layout.setSpacing(8)
        
        self.grid_h_lines_label = QLabel(tr("Horizontal lines"))
        h_lines_layout.addWidget(self.grid_h_lines_label)
        h_lines_layout.addStretch()
        
        # Группа кнопок для горизонтальных линий
        self.h_lines_group = QButtonGroup(self)
        h_lines_buttons = create_grid_buttons(self.h_lines_group, self.current_settings.get("grid_h_lines", 2))
        h_lines_layout.addWidget(h_lines_buttons)
        grid_content_layout.addWidget(h_lines_container)
        
        # Вертикальные линии
        v_lines_container = QWidget()
        v_lines_layout = QHBoxLayout(v_lines_container)
        v_lines_layout.setContentsMargins(0, 0, 0, 0)
        v_lines_layout.setSpacing(8)
        
        self.grid_v_lines_label = QLabel(tr("Vertical lines"))
        v_lines_layout.addWidget(self.grid_v_lines_label)
        v_lines_layout.addStretch()
        
        # Группа кнопок для вертикальных линий
        v_lines_buttons = QWidget()
        v_lines_buttons_layout = QHBoxLayout(v_lines_buttons)
        v_lines_buttons_layout.setContentsMargins(0, 0, 0, 0)
        v_lines_buttons_layout.setSpacing(1)  # Минимальный отступ между кнопками
        
        self.v_lines_group = QButtonGroup(self)
        v_lines_buttons = create_grid_buttons(self.v_lines_group, self.current_settings.get("grid_v_lines", 2))
        v_lines_layout.addWidget(v_lines_buttons)
        grid_content_layout.addWidget(v_lines_container)
        
        right_column_layout.addWidget(grid_content)
        
        # Добавляем растягивающийся элемент в конце правой колонки
        right_column_layout.addStretch()
        
        # Выбор темы
        theme_container = QWidget()
        theme_layout = QHBoxLayout(theme_container)
        theme_layout.setContentsMargins(0, 0, 0, 0)
        theme_layout.setSpacing(8)
        
        theme_label = QLabel(tr("Theme"))
        theme_label.setProperty('isHeader', True)
        theme_label.original_text = "Theme"
        theme_layout.addWidget(theme_label)
        theme_layout.addStretch()
        
        
        self.theme_combo = QComboBox()
        # Добавляем все доступные темы из theme_manager
        for theme in theme_manager._available_themes:
            if theme == "system":
                self.theme_combo.addItem(tr("System"), theme)
            elif theme == "dark":
                self.theme_combo.addItem(tr("Dark"), theme)
            elif theme == "light":
                self.theme_combo.addItem(tr("Light"), theme)
            elif theme == "calcite":
                self.theme_combo.addItem(tr("Calcite"), theme)
            elif theme == "charoite":
                self.theme_combo.addItem(tr("Charoite"), theme)
            elif theme == "emerald":
                self.theme_combo.addItem(tr("Emerald"), theme)
            elif theme == "jasper":
                self.theme_combo.addItem(tr("Jasper"), theme)
            elif theme == "ruby":
                self.theme_combo.addItem(tr("Ruby"), theme)
            elif theme == "sapphire":
                self.theme_combo.addItem(tr("Sapphire"), theme)
        
        
        # Устанавливаем фиксированную ширину для комбобокса
        self.theme_combo.setFixedWidth(140)
        
        # Устанавливаем текущую тему
        current_theme = self.current_settings.get("theme", "dark")
        for i in range(self.theme_combo.count()):
            if self.theme_combo.itemData(i) == current_theme:
                self.theme_combo.setCurrentIndex(i)
                break
                
        colors = theme_manager.get_theme_colors()
        self.theme_combo.setStyleSheet(f"""
            QComboBox {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                border: 1px solid {colors['border']};
                border-radius: 4px;
                padding: 4px 24px 4px 8px;
            }}
            QComboBox:hover {{
                background-color: {colors['background_hover']};
                color: {colors['text']};
                border: 1px solid {colors['border_hover']};
            }}
            QComboBox::drop-down {{
                border: none;
                width: 24px;
            }}
            QComboBox::down-arrow {{
                image: url(resources/down_arrow.png);
                width: 12px;
                height: 12px;
            }}
            QComboBox QAbstractItemView {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                selection-background-color: {colors['accent']};
                selection-color: {colors['text']};
                border: 1px solid {colors['border']};
                padding: 4px 0px;
            }}
        """)
        
        theme_layout.addWidget(self.theme_combo)
        right_column_layout.addWidget(theme_container)
        
        # Селектор языка
        language_container = QWidget()
        language_layout = QHBoxLayout(language_container)
        language_layout.setContentsMargins(0, 0, 0, 0)
        language_layout.setSpacing(8)
        
        language_label = QLabel(tr("Interface language"))
        language_label.setProperty('isHeader', True)
        language_label.original_text = "Interface language"
        language_layout.addWidget(language_label)
        language_layout.addStretch()
        
        self.language_combo = QComboBox()
        for code, name in translation_manager.get_available_languages().items():
            self.language_combo.addItem(name, code)
        
        # Устанавливаем фиксированную ширину для комбобокса
        self.language_combo.setFixedWidth(140)
        
        # Устанавливаем текущий язык
        current_language = self.current_settings.get("language", "en")
        for i in range(self.language_combo.count()):
            if self.language_combo.itemData(i) == current_language:
                self.language_combo.setCurrentIndex(i)
                break
                
        # Используем те же цвета темы для комбобокса языка
        self.language_combo.setStyleSheet(f"""
            QComboBox {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                border: 1px solid {colors['border']};
                border-radius: 4px;
                padding: 4px 24px 4px 8px;
            }}
            QComboBox:hover {{
                background-color: {colors['background_hover']};
                color: {colors['text']};
                border: 1px solid {colors['border_hover']};
            }}
            QComboBox::drop-down {{
                border: none;
                width: 24px;
            }}
            QComboBox::down-arrow {{
                image: url(resources/down_arrow.png);
                width: 12px;
                height: 12px;
            }}
            QComboBox QAbstractItemView {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                selection-background-color: {colors['accent']};
                selection-color: {colors['text']};
                border: 1px solid {colors['border']};
                padding: 4px 0px;
            }}
        """)
        
        language_layout.addWidget(self.language_combo)
        right_column_layout.addWidget(language_container)

        # Добавляем колонки в основной контейнер
        main_settings_layout.addWidget(left_column)
        main_settings_layout.addWidget(right_column)
        
        # Добавляем контейнер с колонками в основной макет
        layout.addWidget(main_settings_container)
        
        # Создаем разделитель
        layout.addWidget(self.create_separator())

        # Контейнер с кнопками внизу
        button_container = QWidget()
        button_layout = QHBoxLayout(button_container)
        button_layout.setContentsMargins(0, 16, 0, 0)  # Уменьшаем верхний отступ
        button_layout.setSpacing(8)  # Уменьшаем отступ между кнопками
        
        # Кнопка сохранения настроек
        self.save_button = QPushButton(tr("Apply settings"))
        self.save_button.clicked.connect(self.apply_settings)
        self.save_button.setStyleSheet(theme_manager.get_theme_styles()["settings_button"])
        
        # Кнопка начала новой сессии
        self.start_session_button = QPushButton(tr("Start new session"))
        self.start_session_button.clicked.connect(self.start_new_session)
        self.start_session_button.setStyleSheet(theme_manager.get_theme_styles()["accent_button"])
        
        button_layout.addStretch()
        button_layout.addWidget(self.save_button)
        button_layout.addWidget(self.start_session_button)
        layout.addWidget(button_container)

    def setup_info_ui(self, layout):
        """Настраивает интерфейс информационной вкладки"""
        # На случай, если словари переводов ещё не были загружены/обновлены
        try:
            translation_manager.load_translations()
        except Exception:
            pass
        # Контейнер с информацией
        info_scroll = QScrollArea()
        info_scroll.setWidgetResizable(True)
        info_scroll.setFrameShape(QFrame.Shape.NoFrame)
        info_scroll.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        info_scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.info_scroll = info_scroll  # Сохраняем для apply_theme
        self.info_content = QWidget()
        info_content_layout = QVBoxLayout(self.info_content)
        info_content_layout.setContentsMargins(0, 0, 0, 0)
        info_content_layout.setSpacing(10)
        
        # Сохраняем ссылки на метки для перевода
        self.info_title_label = QLabel(tr("About GestArt"))
        self.info_title_label.setStyleSheet(f"font-size: 16px; font-weight: bold; color: {theme_manager.get_theme_colors()['text']};")
        info_content_layout.addWidget(self.info_title_label)
        
        self.app_info_label = QLabel(tr("About GestArt Text"))
        self.app_info_label.setWordWrap(True)
        self.app_info_label.setStyleSheet(theme_manager.get_theme_styles()["info_text"])
        info_content_layout.addWidget(self.app_info_label)
        
        self.version_info_label = QLabel(tr("Version") + ": 0.9.8")
        self.version_info_label.setStyleSheet(theme_manager.get_theme_styles()["info_text"])
        info_content_layout.addWidget(self.version_info_label)
        
        info_content_layout.addWidget(self.create_separator())
        
        self.instruction_title_label = QLabel(tr("Instructions"))
        self.instruction_title_label.setStyleSheet(f"font-size: 16px; font-weight: bold; color: {theme_manager.get_theme_colors()['text']};")
        info_content_layout.addWidget(self.instruction_title_label)
        
        self.instructions_label = QLabel(tr("Instructions Text"))
        self.instructions_label.setWordWrap(True)
        self.instructions_label.setStyleSheet(theme_manager.get_theme_styles()["info_text"])
        info_content_layout.addWidget(self.instructions_label)
        
        info_content_layout.addStretch()
        
        self.info_content.setLayout(info_content_layout)
        info_scroll.setWidget(self.info_content)
        layout.addWidget(info_scroll)
        
        layout.addWidget(self.create_separator())

        button_container = QWidget()
        button_layout = QHBoxLayout(button_container)
        button_layout.setContentsMargins(0, 20, 0, 0)
        button_layout.setSpacing(10)
        
        self.close_info_button = QPushButton(tr("Close info"))
        self.close_info_button.clicked.connect(self.toggle_info_interface)
        colors = theme_manager.get_theme_colors()
        self.close_info_button.setStyleSheet(f"""
            QPushButton {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                border: 1px solid {colors['border']};
                border-radius: 6px;
                padding: 10px 16px;
                min-width: 140px;
            }}
            QPushButton:hover {{
                background-color: {colors['background_hover']};
                border: 1px solid {colors['border_hover']};
            }}
            QPushButton:pressed {{
                background-color: {colors['background_pressed']};
            }}
        """)
        
        # Добавляем социальные ссылки
        self.create_social_buttons(button_layout)
        
        button_layout.addStretch()
        button_layout.addWidget(self.close_info_button)
        layout.addWidget(button_container)

    def toggle_info_interface(self):
        """Переключает между интерфейсом настроек и информации"""
        self.show_info_interface = not self.show_info_interface
        
        if self.show_info_interface:
            # Переключаемся на информационный интерфейс
            self.title_label.setText(tr("Information"))
            self.stack_layout.setCurrentIndex(1)  # Индекс информационного виджета
            # Обновляем тексты в информационном интерфейсе
            self.retranslate_ui()
        else:
            # Возвращаемся к интерфейсу настроек
            self.title_label.setText(tr("Settings"))
            self.stack_layout.setCurrentIndex(0)  # Индекс виджета настроек

    def create_separator(self):
        """Создает разделительную линию"""
        line = QFrame()
        line.setFrameShape(QFrame.Shape.HLine)
        line.setStyleSheet(theme_manager.get_theme_styles()["separator"])
        return line

    def create_social_buttons(self, layout):
        """Создает кнопки социальных сетей"""
        
        # Создаем контейнер для социальных кнопок
        social_container = QWidget()
        social_layout = QHBoxLayout(social_container)
        social_layout.setContentsMargins(0, 0, 0, 0)
        social_layout.setSpacing(6)
        
        # Ссылки на социальные сети
        social_links = {
            "GitHub": ("https://github.com/LeonWGal/gestart", "resources/lgithub.png"),
            "Twitter": ("https://twitter.com/LeonWGal", "resources/ltwitter.png"),
            "Telegram": ("https://t.me/LeonWGal", "resources/ltelegram.png"),
            "Discord": ("https://discord.gg/yHrQBTUaGr", "resources/ldiscord.png"),
            "Patreon": ("https://patreon.com/LeonWGal", "resources/lpatreon.png")
        }
        
        # Получаем цвета темы
        colors = theme_manager.get_theme_colors()
        
        # Сохраняем кнопки для обновления стилей
        self.social_buttons = []
        
        for name, (url, icon_path) in social_links.items():
            button = QPushButton()
            button.setFixedSize(28, 28)  # Квадратные кнопки
            button.setIcon(create_themed_icon(icon_path))
            button.setIconSize(QSize(22, 22))  # Квадратные иконки
            button.setToolTip(f"Visit {name}")
            
            # Адаптивные стили в зависимости от темы
            button.setStyleSheet(f"""
                QPushButton {{
                    background-color: transparent;
                    border: none;
                    border-radius: 14px;
                    min-width: 28px;
                    min-height: 28px;
                    max-width: 28px;
                    max-height: 28px;
                }}
                QPushButton:hover {{
                    background-color: {colors['background_hover']};
                }}
                QPushButton:pressed {{
                    background-color: {colors['background_pressed']};
                }}
                QToolTip {{
                    background-color: #000000;
                    color: #ffffff;
                    border: 1px solid #333333;
                    border-radius: 4px;
                    padding: 4px 8px;
                    font-size: 12px;
                }}
            """)
            
            # Подключаем открытие ссылки
            button.clicked.connect(lambda checked, url=url: QDesktopServices.openUrl(QUrl(url)))
            
            social_layout.addWidget(button)
            self.social_buttons.append(button)  # Сохраняем для обновления стилей
        
        layout.addWidget(social_container)

    def connect_signals(self):
        """Подключает все сигналы для элементов управления"""
        # Время показа
        def on_time_slider_value_changed(value):
            if not self.time_adjuster.unlimited_checkbox.isChecked():
                self.time_adjuster.slider.setValue(value)
                
        def on_time_spinbox_value_changed(value):
            if not self.time_adjuster.unlimited_checkbox.isChecked() and value <= 900:
                self.time_adjuster.slider.setValue(value)
                
                
        def on_unlimited_time_toggled(checked):
            self.time_adjuster.slider.setEnabled(not checked)
            self.time_adjuster.slider.setValue(self.time_adjuster.slider.value())
                
        self.time_adjuster.valueChanged.connect(on_time_slider_value_changed)
        self.time_adjuster.unlimited_checkbox.toggled.connect(on_unlimited_time_toggled)

        # Количество изображений
        def on_num_slider_value_changed(value):
            if not self.num_adjuster.unlimited_checkbox.isChecked():
                self.num_adjuster.slider.setValue(value)
                
        def on_num_spinbox_value_changed(value):
            if not self.num_adjuster.unlimited_checkbox.isChecked() and value <= 900:
                self.num_adjuster.slider.setValue(value)
                
        def on_unlimited_num_toggled(checked):
            self.num_adjuster.slider.setEnabled(not checked)
            self.num_adjuster.slider.setValue(self.num_adjuster.slider.value())
                
        self.num_adjuster.valueChanged.connect(on_num_slider_value_changed)
        self.num_adjuster.unlimited_checkbox.toggled.connect(on_unlimited_num_toggled)

        # Перерыв
        def on_break_slider_value_changed(value):
            if not self.break_adjuster.unlimited_checkbox.isChecked():
                self.break_adjuster.slider.setValue(value)
                
        def on_break_spinbox_value_changed(value):
            if not self.break_adjuster.unlimited_checkbox.isChecked() and value <= 900:
                self.break_adjuster.slider.setValue(value)
            
        def on_break_checkbox_toggled(checked):
            self.break_adjuster.slider.setEnabled(checked)
            self.break_adjuster.value_spinbox.setEnabled(checked)
            if checked:
                self.break_adjuster.slider.setValue(self.break_adjuster.slider.value())
            
        self.break_adjuster.valueChanged.connect(on_break_slider_value_changed)
        self.break_checkbox.toggled.connect(on_break_checkbox_toggled)
        
        # Устанавливаем начальное состояние элементов управления перерыва
        # Read only 'use_break'
        self.break_checkbox.setChecked(self.current_settings.get("use_break", False)) 
        self.break_adjuster.slider.setEnabled(self.break_checkbox.isChecked())
        self.break_adjuster.value_spinbox.setEnabled(self.break_checkbox.isChecked())

        # Применение темы в реальном времени при изменении выбора в комбобоксе
        def on_theme_changed(index):
            selected_theme = self.theme_combo.itemData(index)
            if selected_theme != theme_manager.get_current_theme():
                theme_manager.set_theme(selected_theme)
                self.apply_theme()
        
        self.theme_combo.currentIndexChanged.connect(on_theme_changed)

        # Применение языка в реальном времени при изменении выбора в комбобоксе
        def on_language_changed(index):
            selected_language = self.language_combo.itemData(index)
            current_language = self.current_settings.get("language", "en")
            
            # Защита от None значения
            if selected_language is None:
                logging.warning("Selected language is None, using default 'en'")
                selected_language = "en"
            
            if selected_language != current_language:
                # Сохраняем текущие настройки
                self.current_settings["language"] = selected_language
                # Применяем новый язык
                translation_manager.set_locale(selected_language)
                # Обновляем тексты в окне настроек
                self.retranslate_ui()
                # Обновляем тексты в главном окне
                if self.parent():
                    self.parent().retranslate_ui()
    
        self.language_combo.currentIndexChanged.connect(on_language_changed)

    def select_folder(self):
        folder = QFileDialog.getExistingDirectory(self, tr("Select folder"), os.getcwd())
        if folder:
            self.folder_line_edit.setText(folder)

    def delete_history(self):
        """Очищает историю просмотров и сохраняет изменения"""
        if hasattr(self.parent(), 'displayed_history'):
            self.parent().displayed_history.clear()
            if self.parent().settings.get("save_history", True):
                self.parent().config_manager.save_history(self.parent().displayed_history)
            # Очищаем кэш изображений
            if hasattr(self.parent(), 'pixmap_cache'):
                self.parent().pixmap_cache.clear()
            # Обновляем отображение количества изображений в истории
            self.update_history_count()

    def apply_settings(self):
        """Применяет настройки и закрывает диалог"""
        # Получаем значения из групп кнопок для сетки
        h_lines = self.h_lines_group.checkedId()
        v_lines = self.v_lines_group.checkedId()
        
        # Применяем настройки из текущих элементов управления
        settings = {
            "folder": self.current_settings.get("folder", ""),
            "display_time": self.time_adjuster.value(),
            "num_images": self.num_adjuster.value(),
            "break_duration": self.break_adjuster.value() * 60,  # Конвертируем минуты в секунды
            "unlimited_time": self.time_adjuster.unlimited_checkbox.isChecked(),
            "unlimited_images": self.num_adjuster.unlimited_checkbox.isChecked(),
            "use_break": self.break_checkbox.isChecked(),  # Save only 'use_break'
            "enable_breaks": self.break_checkbox.isChecked(),  # Для совместимости оставляем оба ключа
            "timer_position": self.timer_position.currentPosition(),
            "save_history": self.history_checkbox.isChecked(),
            "confirm_delete": self.confirm_delete_checkbox.isChecked(),
            "show_timer": self.current_settings.get("show_timer", True),
            "always_on_top": self.current_settings.get("always_on_top", False),
            "preview_mode": self.folder_drop.preview_mode_btn.isChecked(),
            "language": self.language_combo.currentData(),
            "grid_h_lines": h_lines if h_lines > 0 else 2,  # Значение по умолчанию 2
            "grid_v_lines": v_lines if v_lines > 0 else 2,  # Значение по умолчанию 2
            "timer_volume": self.timer_volume_adjuster.value(),
            "theme": self.theme_combo.currentData()
        }
        
        # Эмитим сигнал с новыми настройками
        self.settings_updated.emit(settings)
        self.accept()
    
    def retranslate_ui(self):
        """Обновляет все тексты интерфейса при изменении языка"""
        # Обновляем словари переводов на случай изменений на диске
        try:
            translation_manager.load_translations()
        except Exception:
            pass
        # Обновляем заголовок окна
        self.setWindowTitle(tr("Settings"))
        
        # Обновляем заголовки разделов
        self.title_label.setText(tr("Settings") if not self.show_info_interface else tr("Information"))
        
        # Обновляем тексты в информационном разделе, используя сохраненные ссылки
        if hasattr(self, 'info_title_label'):
            self.info_title_label.setText(tr("About GestArt"))
        if hasattr(self, 'app_info_label'):
            self.app_info_label.setText(tr("About GestArt Text"))
        if hasattr(self, 'version_info_label'):
            self.version_info_label.setText(tr("Version") + ": 0.9.8")
        if hasattr(self, 'instruction_title_label'):
            self.instruction_title_label.setText(tr("Instructions"))
        if hasattr(self, 'instructions_label'):
            self.instructions_label.setText(tr("Instructions Text"))

        # Обновляем тексты кнопок
        self.close_info_button.setText(tr("Close info"))
        self.save_button.setText(tr("Apply settings"))
        self.start_session_button.setText(tr("Start new session"))
        self.close_button.setToolTip(tr("Close"))
        self.info_button.setToolTip(tr("Information"))
        
        # Обновляем тексты в основном интерфейсе настроек
        if hasattr(self, 'current_folder_label'):
            if self.current_settings.get("folder"):
                folder_info = f"{tr('Current folder')}: {self.current_settings['folder']}"
                total_images = len(self.parent().image_files) if hasattr(self.parent(), 'image_files') else 0
                images_info = f"\n{tr('Images in folder')}: {total_images}"
                self.current_folder_label.setText(folder_info + images_info)
            else:
                self.current_folder_label.setText(tr("No folder selected"))

        # Обновляем тексты в области перетаскивания
        if hasattr(self, 'folder_drop'):
            self.folder_drop.standard_mode_btn.setText(tr("Standard"))
            self.folder_drop.preview_mode_btn.setText(tr("Preview"))
            if not self.current_settings.get("folder"):
                self.folder_drop.standard_drop.setText(tr("Drop folder here or click to select"))
            # Принудительно обновляем стили кнопок режима
            self.folder_drop.apply_theme()

        # Обновляем заголовки секций
        for label in self.findChildren(QLabel):
            if hasattr(label, 'property') and label.property('isHeader'):
                # Используем сохраненный оригинальный ключ для перевода
                if hasattr(label, 'original_text'):
                    label.setText(tr(label.original_text))
                else:
                    # Если оригинальный текст не сохранен, пытаемся определить по текущему тексту
                    text = label.text()
                    # Словарь соответствий для обратной совместимости
                    text_mapping = {
                        "Session settings": "Session settings",
                        "Настройки сессии": "Session settings", 
                        "Сессия туруоруулара": "Session settings",
                        "Break settings": "Break settings",
                        "Настройки перерыва": "Break settings",
                        "Араас туруоруулара": "Break settings",
                        "Timer Sound": "Timer Sound",
                        "Звук таймера": "Timer Sound",
                        "Таймер тавышы": "Timer Sound",
                        "History": "History",
                        "История": "History",
                        "Тарих": "History",
                        "Grid Settings": "Grid Settings",
                        "Настройки сетки": "Grid Settings",
                        "Theme": "Theme",
                        "Тема": "Theme",
                        "Interface language": "Interface language",
                        "Язык интерфейса": "Interface language",
                        "Интерфейс тыла": "Interface language",
                        "Timer position": "Timer position",
                        "Позиция таймера": "Timer position",
                        "Таймер сирэ": "Timer position"
                    }
                    if text in text_mapping:
                        label.setText(tr(text_mapping[text]))

        # Обновляем заголовки и метки
        for widget in self.findChildren(QWidget):
            if hasattr(widget, 'title') and isinstance(widget.title, str):
                widget.title = tr(widget.title)
            if hasattr(widget, 'label') and isinstance(widget.label, QLabel):
                original_text = widget.label.text()
                if original_text:
                    widget.label.setText(tr(original_text))

        # Обновляем тексты в ValueAdjuster
        if hasattr(self, 'time_adjuster'):
            self.time_adjuster.label.setText(tr("Display time (sec):"))
            self.time_adjuster.unlimited_checkbox.setToolTip(tr("Unlimited"))
        if hasattr(self, 'num_adjuster'):
            self.num_adjuster.label.setText(tr("Number of images:"))
            self.num_adjuster.unlimited_checkbox.setToolTip(tr("Unlimited"))
        if hasattr(self, 'break_adjuster'):
            self.break_adjuster.label.setText(tr("Duration (min):"))
            self.break_checkbox.setText(tr("Enable breaks"))
        if hasattr(self, 'timer_volume_adjuster'):
            self.timer_volume_adjuster.label.setText(tr("Volume:"))

        # Обновляем тексты в истории
        if hasattr(self, 'history_checkbox'):
            self.history_checkbox.setText(tr("Save viewed images"))
            self.delete_history_button.setText(tr("Clear history"))
            self.confirm_delete_checkbox.setText(tr("Confirm file deletion"))
            self.update_history_count()

        # Обновляем тексты в настройках сетки
        if hasattr(self, 'grid_settings_label'):
            self.grid_settings_label.setText(tr("Grid Settings"))
            self.grid_h_lines_label.setText(tr("Horizontal lines"))
            self.grid_v_lines_label.setText(tr("Vertical lines"))

        # Обновляем тексты в селекторе позиции таймера
        if hasattr(self, 'timer_position') and hasattr(self.timer_position, 'combo'):
            current_position = self.timer_position.combo.currentData()
            
            # Блокируем сигнал перед обновлением
            self.timer_position.combo.blockSignals(True)
            
            self.timer_position.combo.clear()
            self.timer_position.combo.addItem(tr("Left"), "left")
            self.timer_position.combo.addItem(tr("Center"), "center")
            self.timer_position.combo.addItem(tr("Right"), "right")
            
            # Восстанавливаем выбранную позицию
            for i in range(self.timer_position.combo.count()):
                if self.timer_position.combo.itemData(i) == current_position:
                    self.timer_position.combo.setCurrentIndex(i)
                    break
            
            # Разблокируем сигнал после обновления
            self.timer_position.combo.blockSignals(False)

        # Обновляем тексты в комбобоксах
        if hasattr(self, 'theme_combo'):
            current_theme = self.theme_combo.currentData()
            
            # Блокируем сигнал перед обновлением
            self.theme_combo.blockSignals(True)
            
            self.theme_combo.clear()
            # Добавляем все доступные темы из theme_manager
            for theme in theme_manager._available_themes:
                if theme == "system":
                    self.theme_combo.addItem(tr("System"), theme)
                elif theme == "dark":
                    self.theme_combo.addItem(tr("Dark"), theme)
                elif theme == "light":
                    self.theme_combo.addItem(tr("Light"), theme)
                elif theme == "calcite":
                    self.theme_combo.addItem(tr("Calcite"), theme)
                elif theme == "charoite":
                    self.theme_combo.addItem(tr("Charoite"), theme)
                elif theme == "emerald":
                    self.theme_combo.addItem(tr("Emerald"), theme)
                elif theme == "jasper":
                    self.theme_combo.addItem(tr("Jasper"), theme)
                elif theme == "ruby":
                    self.theme_combo.addItem(tr("Ruby"), theme)
                elif theme == "sapphire":
                    self.theme_combo.addItem(tr("Sapphire"), theme)
            
            # Восстанавливаем выбранную тему
            for i in range(self.theme_combo.count()):
                if self.theme_combo.itemData(i) == current_theme:
                    self.theme_combo.setCurrentIndex(i)
                    break
            
            # Разблокируем сигнал после обновления
            self.theme_combo.blockSignals(False)

        # Обновляем тексты в языковом комбобоксе
        if hasattr(self, 'language_combo'):
            current_language = self.language_combo.currentData()
            # Защита от None значения
            if current_language is None:
                current_language = "en"
                
            # Блокируем сигнал перед обновлением
            self.language_combo.blockSignals(True)
            
            # Обновляем содержимое - используем все доступные языки
            self.language_combo.clear()
            for code, name in translation_manager.get_available_languages().items():
                self.language_combo.addItem(name, code)
            
            # Восстанавливаем выбранный язык
            for i in range(self.language_combo.count()):
                if self.language_combo.itemData(i) == current_language:
                    self.language_combo.setCurrentIndex(i)
                    break
            
            # Разблокируем сигнал после обновления
            self.language_combo.blockSignals(False)

    def apply_theme(self):
        """Применяет текущую тему к диалогу настроек"""
        # Получаем подходящий шрифт и размеры для текущего языка
        current_language = self.current_settings.get("language", "en")
        font_family = get_font_family_for_language(current_language)
        font_sizes = get_font_size_for_language(current_language)
        
        styles = theme_manager.get_theme_styles(font_family, font_sizes)
        colors = theme_manager.get_theme_colors()
        
        # Применяем стиль к основному контейнеру
        self.container.setStyleSheet(styles["settings_container"])
        
        # Применяем стиль к области перетаскивания папки
        self.folder_drop.standard_drop.setStyleSheet(styles["folder_drop"])
        self.folder_drop.preview_drop.setStyleSheet(styles["folder_drop"])
        
        # Применяем тему к области перетаскивания (включая кнопки режима)
        self.folder_drop.apply_theme()
        
        # Применяем стиль к метке текущей папки
        self.current_folder_label.setStyleSheet(f"""
            color: {colors['text_secondary']};
            padding: 8px;
            background-color: {colors['background_secondary']};
            border-radius: 8px;
        """)
        
        # Применяем стиль к заголовкам и меткам
        for label in self.findChildren(QLabel):
            if hasattr(label, 'property') and label.property('isHeader'):
                label.setStyleSheet(f"color: {colors['text']}; font-weight: bold; font-size: {font_sizes['header']}px; margin-bottom: 6px; font-family: {font_family};")
            else:
                label.setStyleSheet(f"color: {colors['text']}; font-family: {font_family};")
        
        # Обновляем стили для слайдеров
        self.time_adjuster.apply_theme()
        self.num_adjuster.apply_theme()
        self.break_adjuster.apply_theme()
        self.timer_volume_adjuster.apply_theme()
        
        # Создаем стиль для чекбоксов
        checkbox_style = f"""
            QCheckBox {{
                color: {colors['text']};
                font-size: 14px;
                spacing: 8px;
            }}
            QCheckBox::indicator {{
                width: 18px;
                height: 18px;
                border-radius: 4px;
                border: 1px solid {colors['border']};
                background: {colors['background_secondary']};
            }}
            QCheckBox::indicator:hover {{
                border: 1px solid {colors['border_hover']};
                background: {colors['background_hover']};
            }}
            QCheckBox::indicator:checked {{
                background: {colors['background_checked']};
                border: 1px solid {colors['border_hover']};
            }}
        """
        
        # Применяем стиль ко всем чекбоксам
        for checkbox in self.findChildren(QCheckBox):
            checkbox.setStyleSheet(checkbox_style)
        
        # Обновляем стили для кнопок сетки
        grid_button_style = f"""
                    QPushButton {{
                        background-color: {colors['background_secondary']};
                        color: {colors['text']};
                        border: 1px solid {colors['border']};
                        border-radius: 3px;
                        padding: 0px;
                        font-size: 14px;
                        min-width: 40px;
                        max-width: 40px;
                    }}
                    QPushButton:hover {{
                        background-color: {colors['background_hover']};
                        border: 1px solid {colors['border_hover']};
                    }}
                    QPushButton:checked {{
                        background-color: {colors['background_checked']};
                        border: 1px solid {colors['border_focus']};
                    }}
        """
        
        for group in [self.h_lines_group, self.v_lines_group]:
            for button in group.buttons():
                button.setStyleSheet(grid_button_style)
        
        # Обновляем стили для комбобоксов
        combobox_style = f"""
            QComboBox {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                border: 1px solid {colors['border']};
                border-radius: 4px;
                padding: 4px 24px 4px 8px;
            }}
            QComboBox:hover {{
                background-color: {colors['background_hover']};
                color: {colors['text']};
                border: 1px solid {colors['border_hover']};
            }}
            QComboBox::drop-down {{
                border: none;
                width: 24px;
            }}
            QComboBox::down-arrow {{
                image: url(resources/down_arrow.png);
                width: 12px;
                height: 12px;
            }}
            QComboBox QAbstractItemView {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                selection-background-color: {colors['accent']};
                selection-color: {colors['text']};
                border: 1px solid {colors['border']};
                padding: 4px 0px;
            }}
        """
        self.theme_combo.setStyleSheet(combobox_style)
        self.language_combo.setStyleSheet(combobox_style)
        self.timer_position.combo.setStyleSheet(combobox_style)
        
        # Обновляем стиль для кнопок
        self.save_button.setStyleSheet(styles["settings_button"])
        self.start_session_button.setStyleSheet(styles["accent_button"])
        
        # Обновляем стиль для кнопок info и close
        header_button_style = f"""
            QPushButton {{
                background-color: transparent;
                border: none;
                padding: 0;
            }}
            QPushButton:hover {{
                background-color: {colors['background_hover']};
                border-radius: 4px;
            }}
        """
        self.info_button.setStyleSheet(header_button_style)
        self.close_button.setStyleSheet(header_button_style)
        
        # Обновляем иконки
        self.close_button.setIcon(create_themed_icon("resources/exit.png"))
        self.info_button.setIcon(create_themed_icon("resources/info.png"))
        
        # Обновляем разделители
        for separator in self.findChildren(QFrame):
            if separator.frameShape() == QFrame.Shape.HLine:
                separator.setStyleSheet(styles["separator"])
                
        # Обновляем отображение
        self.update()

        # Фон информационного окна — как у настроек
        bg_color = colors['background_secondary']
        self.info_widget.setStyleSheet(f"background: {bg_color};")
        if hasattr(self, 'info_scroll'):
            # Современный стиль скроллбара с градиентом и плавным затемнением
            scrollbar_style = f'''
            QScrollBar:vertical {{
                background: transparent;
                width: 8px;
                margin: 4px 0 4px 0;
                border-radius: 4px;
            }}
            QScrollBar::handle:vertical {{
                background: qlineargradient(
                    x1:0, y1:0, x2:1, y2:0,
                    stop:0 {colors['accent']}, stop:1 {colors['border_hover']}
                );
                min-height: 32px;
                border-radius: 4px;
                transition: background 0.2s;
            }}
            QScrollBar::handle:vertical:hover {{
                background: qlineargradient(
                    x1:0, y1:0, x2:1, y2:0,
                    stop:0 {colors['border_hover']}, stop:1 {colors['accent']}
                );
            }}
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
                height: 0;
                background: none;
                border: none;
            }}
            QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {{
                background: none;
            }}
            QScrollBar:horizontal {{
                background: transparent;
                height: 8px;
                margin: 0 4px 0 4px;
                border-radius: 4px;
            }}
            QScrollBar::handle:horizontal {{
                background: qlineargradient(
                    x1:0, y1:0, x2:0, y2:1,
                    stop:0 {colors['accent']}, stop:1 {colors['border_hover']}
                );
                min-width: 32px;
                border-radius: 4px;
                transition: background 0.2s;
            }}
            QScrollBar::handle:horizontal:hover {{
                background: qlineargradient(
                    x1:0, y1:0, x2:0, y2:1,
                    stop:0 {colors['border_hover']}, stop:1 {colors['accent']}
                );
            }}
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {{
                width: 0;
                background: none;
                border: none;
            }}
            QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {{
                background: none;
            }}
            '''
            self.info_scroll.setStyleSheet(f"background: {bg_color};" + scrollbar_style)
        if hasattr(self, 'info_content'):
            self.info_content.setStyleSheet(f"background: {bg_color};")
        
        # Обновляем стиль кнопки "Close Info"
        if hasattr(self, 'close_info_button'):
            close_info_style = f"""
                QPushButton {{
                    background-color: {colors['background_secondary']};
                    color: {colors['text']};
                    border: 1px solid {colors['border']};
                    border-radius: 6px;
                    padding: 8px 16px;
                    font-size: 14px;
                    font-weight: bold;
                }}
                QPushButton:hover {{
                    background-color: {colors['background_hover']};
                    border: 1px solid {colors['border_hover']};
                }}
                QPushButton:pressed {{
                    background-color: {colors['background_pressed']};
                }}
            """
            self.close_info_button.setStyleSheet(close_info_style)
            
            # Обновляем стили социальных кнопок
            if hasattr(self, 'social_buttons'):
                for button in self.social_buttons:
                    button.setStyleSheet(f"""
                        QPushButton {{
                            background-color: transparent;
                            border: none;
                            border-radius: 14px;
                            min-width: 28px;
                            min-height: 28px;
                            max-width: 28px;
                            max-height: 28px;
                        }}
                        QPushButton:hover {{
                            background-color: {colors['background_hover']};
                        }}
                        QPushButton:pressed {{
                            background-color: {colors['background_pressed']};
                        }}
                        QToolTip {{
                            background-color: #000000;
                            color: #ffffff;
                            border: 1px solid #333333;
                            border-radius: 4px;
                            padding: 4px 8px;
                            font-size: 12px;
                        }}
                    """)

    def mousePressEvent(self, event):
        """Обработка нажатия мыши для перетаскивания окна"""
        if event.button() == Qt.MouseButton.LeftButton:
            self._drag_pos = event.pos()
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event):
        """Обработка перемещения окна"""
        if hasattr(self, '_drag_pos'):
            delta = event.pos() - self._drag_pos
            self.move(self.pos() + delta)

    def mouseReleaseEvent(self, event):
        """Обработка отпускания кнопки мыши"""
        if hasattr(self, '_drag_pos'):
            del self._drag_pos

    def update_folder_label(self, folder_path):
        if folder_path:
            # Получаем абсолютный путь к папке
            abs_folder_path = os.path.abspath(folder_path)
            
            # Получаем количество изображений в папке
            total_images = len(self.parent().image_files) if hasattr(self.parent(), 'image_files') else 0
            
            # Формируем текст с информацией о папке и количестве изображений
            folder_info = f"{tr('Current folder')}: {folder_path}"
            images_info = f"\n{tr('Images in folder')}: {total_images}"
            
            self.current_folder_label.setText(folder_info + images_info)
            self.current_folder_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.folder_drop.set_folder(folder_path)
        else:
            self.current_folder_label.setText(tr("No folder selected"))
            self.current_folder_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.folder_drop.clear_folder()

    def on_folder_dropped(self, folder_path):
        if os.path.isdir(folder_path):
            self.current_settings["folder"] = folder_path
            self.update_folder_label(folder_path)
            
            # Если это режим перетащенных изображений, обновляем превью
            if hasattr(self, 'is_dropped_images_mode') and self.is_dropped_images_mode:
                self.load_preview_images(folder_path)
            # Загружаем изображения для превью, но не запускаем сессию
            if hasattr(self.parent(), 'load_images'):
                self.parent().scanner_thread = FolderScannerThread(folder_path, self.parent().supported_extensions)
                self.parent().scanner_thread.scanned.connect(self.on_preview_images_scanned)
                self.parent().scanner_thread.error.connect(self.parent().on_scanner_error)
                self.parent().scanner_thread.progress.connect(self.parent().on_progress)
                self.parent().scanner_thread.start()

    def on_preview_images_scanned(self, files):
        if hasattr(self.parent(), 'progress_label'):
            self.parent().progress_label.hide()
        if files:
            self.parent().image_files = files
            # Логируем количество найденных изображений
            logging.info(f"Found images: {len(files)}")
            
            # Если это режим перетащенных изображений, настраиваем превью
            if hasattr(self, 'is_dropped_images_mode') and self.is_dropped_images_mode:
                self.preview_images = files
                if self.preview_mode_btn.isChecked():
                    # Ограничиваем количество превью для лучшей производительности
                    if len(self.preview_images) > 15:
                        self.preview_images = random.sample(self.preview_images, 15)
                    else:
                        random.shuffle(self.preview_images)
                    
                    # Обновляем UI для отображения превью
                    colors = theme_manager.get_theme_colors()
                    self.preview_drop.setStyleSheet(f"""
                        QLabel {{
                            background-color: transparent;
                            border: 2px dashed {colors['border']};
                            border-radius: 8px;
                            padding: 0px;
                            color: {colors['text_secondary']};
                        }}
                    """)
                    
                    # Запускаем превью
                    self.preview_timer.start()
                    self.update_preview_image()
            
            # Обновляем информацию о количестве изображений в текущей папке
            current_folder = self.current_settings.get("folder", "")
            if current_folder:
                self.update_folder_label(current_folder)

    def on_mode_changed(self, preview_mode):
        # Обновляем текущие настройки при изменении режима
        self.current_settings["preview_mode"] = preview_mode
        # Не отправляем сигнал settings_updated, так как это вызовет начало новой сессии
        # self.settings_updated.emit(self.current_settings)
        
    def start_new_session(self):
        """Начать новую сессию с текущими настройками"""
        # Получаем значения из групп кнопок для сетки
        h_lines = self.h_lines_group.checkedId()
        v_lines = self.v_lines_group.checkedId()
        
        # Получаем настройки из текущих элементов управления
        settings = {
            "folder": self.current_settings.get("folder", ""),
            "display_time": self.time_adjuster.value(),
            "num_images": self.num_adjuster.value(),
            "break_duration": self.break_adjuster.value() * 60,  # Конвертируем минуты в секунды
            "unlimited_time": self.time_adjuster.unlimited_checkbox.isChecked(),
            "unlimited_images": self.num_adjuster.unlimited_checkbox.isChecked(),
            "use_break": self.break_checkbox.isChecked(),  # Save only 'use_break'
            "enable_breaks": self.break_checkbox.isChecked(),  # Для совместимости оставляем оба ключа
            "timer_position": self.timer_position.currentPosition(),
            "save_history": self.history_checkbox.isChecked(),
            "confirm_delete": self.confirm_delete_checkbox.isChecked(),
            "show_timer": self.current_settings.get("show_timer", True),
            "always_on_top": self.current_settings.get("always_on_top", False),
            "preview_mode": self.folder_drop.preview_mode_btn.isChecked(),
            "language": self.language_combo.currentData(),
            "grid_h_lines": h_lines if h_lines > 0 else 2,  # Значение по умолчанию 2
            "grid_v_lines": v_lines if v_lines > 0 else 2,  # Значение по умолчанию 2
            "timer_volume": self.timer_volume_adjuster.value(),
            "theme": self.theme_combo.currentData()
        }
        
        # Передаем настройки в MainWindow
        self.settings_updated.emit(settings)
        
        # Запускаем новую сессию в родительском окне, если такой метод существует
        if hasattr(self.parent(), 'start_new_session'):
            self.parent().start_new_session()
            
        self.accept()

    def apply_image_effects(self, pixmap):
        # Здесь оставляем пустую реализацию, так как эффекты применяются в MainWindow
        return pixmap

    def toggle_pause(self):
        """Переключение режима паузы"""
        self.is_paused = not self.is_paused
        if self.is_paused:
            if self.countdown_timer:
                self.countdown_timer.stop()
            self.pause_button.setIcon(create_themed_icon("resources/play0.png"))
            self.pause_button.setToolTip(tr("Resume") + " (Space)")
            
            # Останавливаем звук таймера, если он играет
            if self.timer_sound_playing:
                self.timer_sound_player.stop()
                self.timer_sound_playing = False
        else:
            if not self.settings.get("unlimited_time", False):
                if self.countdown_timer:
                    self.countdown_timer.start()
                self.pause_button.setIcon(create_themed_icon("resources/pause0.png"))
                self.pause_button.setToolTip(tr("Pause") + " (Space)")

    def dragEnterEvent(self, event):
        """Обработка начала перетаскивания"""
        if event.mimeData().hasUrls():
            urls = event.mimeData().urls()
            # Проверяем, есть ли папки или изображения
            has_folder = any(os.path.isdir(url.toLocalFile()) for url in urls)
            has_images = any(self.is_image_file(url.toLocalFile()) for url in urls)
            
            if has_folder or has_images:
                event.acceptProposedAction()
                return
        event.ignore()

    def dropEvent(self, event):
        """Обработка сброса файлов/папок"""
        if event.mimeData().hasUrls():
            urls = event.mimeData().urls()
            folders = []
            images = []
            
            for url in urls:
                file_path = url.toLocalFile()
                if os.path.isdir(file_path):
                    folders.append(file_path)
                elif self.is_image_file(file_path):
                    images.append(file_path)
            
            # Приоритет: папки, затем изображения
            if folders:
                # Если есть папки, используем первую
                self.on_folder_dropped(folders[0])
                event.acceptProposedAction()
            elif images:
                # Если есть изображения, обрабатываем их
                self.handle_dropped_images(images)
                event.acceptProposedAction()
            else:
                event.ignore()
        else:
            event.ignore()
            
    def handle_dropped_images(self, image_paths):
        """Обрабатывает перетащенные изображения, создавая временную папку"""
        try:
            import tempfile
            import shutil
            
            # Создаем временную папку
            temp_dir = tempfile.mkdtemp(prefix="gestart_images_")
            
            # Копируем изображения во временную папку
            for i, image_path in enumerate(image_paths):
                if os.path.exists(image_path):
                    filename = os.path.basename(image_path)
                    # Добавляем номер, если файл с таким именем уже существует
                    base_name, ext = os.path.splitext(filename)
                    new_filename = f"{base_name}_{i:03d}{ext}"
                    dest_path = os.path.join(temp_dir, new_filename)
                    shutil.copy2(image_path, dest_path)
            
            # Сохраняем список перетащенных изображений
            self.dropped_images = image_paths
            self.is_dropped_images_mode = True
            
            # Используем временную папку как обычную папку
            self.on_folder_dropped(temp_dir)
            
        except Exception as e:
            logging.error(f"Error handling dropped images: {e}")
            
    def clear_dropped_images_mode(self):
        """Очищает режим работы с перетащенными изображениями"""
        self.dropped_images = []
        self.is_dropped_images_mode = False

    def accept(self):
        # Stop animations before accepting
        if hasattr(self, 'folder_drop') and self.folder_drop:
            if hasattr(self.folder_drop, 'preview_timer'):
                self.folder_drop.preview_timer.stop()
            if hasattr(self.folder_drop, 'preview_animation'):
                self.folder_drop.preview_animation.stop()
        super().accept()
        
    def reject(self):
        # Stop animations before rejecting
        if hasattr(self, 'folder_drop') and self.folder_drop:
            if hasattr(self.folder_drop, 'preview_timer'):
                self.folder_drop.preview_timer.stop()
            if hasattr(self.folder_drop, 'preview_animation'):
                self.folder_drop.preview_animation.stop()
        super().reject()

# Добавляем новый класс для области перетаскивания
class FolderDropArea(QWidget):
    folder_dropped = pyqtSignal(str)
    mode_changed = pyqtSignal(bool)  # Сигнал для оповещения об изменении режима (True = preview mode)

    def __init__(self, parent=None):
        super().__init__(parent)
        
        # Инициализируем все переменные в начале
        self._preview_offset = 0
        self.preview_images = []
        self.current_preview_index = 0
        self.current_preview_pixmap = None
        self.preview_direction = 1  # 1 = вверх, -1 = вниз
        self.preview_cache = {}  # Кэш для превью изображений
        self.max_preview_cache = 10  # Максимум 10 превью в кэше
        self.dropped_images = []  # Список перетащенных изображений
        self.is_dropped_images_mode = False  # Режим работы с перетащенными изображениями
        
        # Включаем поддержку drag & drop
        self.setAcceptDrops(True)
        
        # Создаем таймер и анимацию (более быстрые и ровные)
        self.preview_timer = QTimer(self)
        self.preview_timer.timeout.connect(self.update_preview_image)
        self.preview_timer.setInterval(9000)  # быстрее переключение превью
        
        self.preview_animation = QPropertyAnimation(self, b"preview_offset")
        self.preview_animation.setDuration(7000)  # быстрее прокрутка
        self.preview_animation.setLoopCount(-1)  # Бесконечное повторение
        self.preview_animation.setEasingCurve(QEasingCurve.Type.Linear)  # ровная скорость
        
        # Создаем layout
        self.layout = QVBoxLayout(self)
        self.layout.setContentsMargins(0, 0, 0, 0)
        self.layout.setSpacing(0)

        # Контейнер для режимов
        self.mode_container = QWidget()
        self.mode_layout = QHBoxLayout(self.mode_container)
        self.mode_layout.setContentsMargins(0, 0, 0, 4)
        self.mode_layout.setSpacing(4)

        # Переключатель режимов
        self.mode_group = QButtonGroup(self)
        self.standard_mode_btn = QPushButton(tr("Standard"))
        self.preview_mode_btn = QPushButton(tr("Preview"))
        self.standard_mode_btn.setCheckable(True)
        self.preview_mode_btn.setCheckable(True)
        self.mode_group.addButton(self.standard_mode_btn)
        self.mode_group.addButton(self.preview_mode_btn)
        
        # Устанавливаем режим из настроек
        if parent and hasattr(parent, "current_settings"):
            if parent.current_settings.get("preview_mode", False):
                self.preview_mode_btn.setChecked(True)
            else:
                self.standard_mode_btn.setChecked(True)
        else:
            self.standard_mode_btn.setChecked(True)

        self.mode_layout.addWidget(self.standard_mode_btn)
        self.mode_layout.addStretch()
        self.mode_layout.addWidget(self.preview_mode_btn)

        self.layout.addWidget(self.mode_container)

        # Стандартный режим
        self.standard_drop = QLabel()
        self.standard_drop.setText(tr("Click the button below or drag a folder here"))
        self.standard_drop.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.standard_drop.setAcceptDrops(True)
        self.standard_drop.setCursor(Qt.CursorShape.PointingHandCursor)
        self.standard_drop.setFixedHeight(160)

        # Режим предпросмотра
        self.preview_drop = QLabel()
        self.preview_drop.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.preview_drop.setAcceptDrops(True)
        self.preview_drop.setCursor(Qt.CursorShape.PointingHandCursor)
        self.preview_drop.setFixedHeight(160)
        self.preview_drop.hide()

        self.layout.addWidget(self.standard_drop)
        self.layout.addWidget(self.preview_drop)

        # Подключаем сигналы
        self.mode_group.buttonClicked.connect(self.on_mode_changed)
        self.standard_drop.mousePressEvent = self.on_click
        # Правый клик на превью — сразу открыть текущий файл, без меню
        self.preview_drop.mousePressEvent = self.on_preview_click
        self.preview_drop.setContextMenuPolicy(Qt.ContextMenuPolicy.NoContextMenu)
        
        # Применяем начальный стиль после подключения сигналов
        self.apply_initial_style()
        
        # Применяем тему для корректного отображения границ
        self.apply_theme()
        
        # Принудительно обновляем отображение для корректной отрисовки рамки
        QTimer.singleShot(0, self.force_style_update)
        
    def force_style_update(self):
        """Принудительно обновляет стили для корректной отрисовки рамки"""
        self.standard_drop.repaint()
        self.preview_drop.repaint()
        self.update()
        
    def on_preview_click(self, event):
        """ЛКМ — выбор папки, ПКМ — открыть текущее изображение без меню"""
        if event.button() == Qt.MouseButton.RightButton:
            self.open_preview_image()
            return
        if event.button() == Qt.MouseButton.LeftButton:
            self.on_click(event)
            return
        
    def open_preview_image(self):
        """Открывает текущее изображение из превью в системном просмотрщике"""
        if not self.preview_images or self.current_preview_index >= len(self.preview_images):
            return
            
        image_path = self.preview_images[self.current_preview_index]
        try:
            import subprocess
            import platform
            
            if platform.system() == "Windows":
                os.startfile(image_path)
            elif platform.system() == "Darwin":  # macOS
                subprocess.run(["open", image_path])
            else:  # Linux
                subprocess.run(["xdg-open", image_path])
        except Exception as e:
            logging.error(f"Error opening preview image: {e}")
            
    def open_preview_folder(self):
        """Открывает папку с текущим изображением из превью"""
        if not self.preview_images or self.current_preview_index >= len(self.preview_images):
            return
            
        image_path = self.preview_images[self.current_preview_index]
        folder_path = os.path.dirname(image_path)
        
        try:
            import subprocess
            import platform
            
            if platform.system() == "Windows":
                os.startfile(folder_path)
            elif platform.system() == "Darwin":  # macOS
                subprocess.run(["open", folder_path])
            else:  # Linux
                subprocess.run(["xdg-open", folder_path])
        except Exception as e:
            logging.error(f"Error opening preview folder: {e}")
            
    def is_image_file(self, file_path):
        """Проверяет, является ли файл изображением"""
        if not os.path.isfile(file_path):
            return False
        supported_extensions = (".png", ".jpg", ".jpeg", ".bmp", ".gif", ".webp", ".tiff", ".tif", ".ico", ".svg", ".heic", ".heif")
        return file_path.lower().endswith(supported_extensions)
        
    def apply_initial_style(self):
        """Применяет начальный стиль для области перетаскивания"""
        colors = theme_manager.get_theme_colors()
        
        # Стиль для стандартной области (с отступом для текста)
        standard_style = f"""
            QLabel {{
                background-color: transparent;
                border: 2px dashed {colors['border']};
                border-radius: 8px;
                padding: 10px; /* Отступ для текста */
                color: {colors['text_secondary']};
                font-size: 12px;
            }}
            QLabel:hover {{
                border-color: {colors['border_hover']};
                color: {colors['text']};
            }}
        """
        
        # Стиль для области предпросмотра (без отступа)
        preview_style = f"""
            QLabel {{
                background-color: transparent;
                border: 2px dashed {colors['border']};
                border-radius: 8px;
                padding: 0px; /* Без отступа для изображения */
                color: {colors['text_secondary']}; /* Цвет на всякий случай */
                font-size: 12px;
            }}
            QLabel:hover {{
                border-color: {colors['border_hover']};
            }}
        """
        
        self.standard_drop.setStyleSheet(standard_style)
        self.preview_drop.setStyleSheet(preview_style)

    def apply_theme(self):
        """Применяет текущую тему к элементам интерфейса"""
        styles = theme_manager.get_theme_styles()
        colors = theme_manager.get_theme_colors()
        
        # Стиль для кнопок режима
        mode_button_style = f"""
            QPushButton {{
                background-color: {colors['background_secondary']};
                color: {colors['text_secondary']};
                border: 1px solid {colors['border']};
                border-radius: 4px;
                padding: 4px 8px;
                font-size: 11px;
                min-width: 60px;
            }}
            QPushButton:hover {{
                background-color: {colors['background_hover']};
                color: {colors['text']};
                border: 1px solid {colors['border_hover']};
            }}
            QPushButton:checked {{
                background-color: {colors['accent']};
                color: {colors['text']};
                border: 1px solid {colors['accent']};
            }}
        """
        
        # Стиль для выбранной кнопки
        selected_button_style = f"""
            QPushButton {{
                background-color: {colors['accent']};
                color: {colors['text']};
                border: 1px solid {colors['accent']};
                border-radius: 4px;
                padding: 4px 8px;
                font-size: 11px;
                min-width: 60px;
            }}
            QPushButton:hover {{
                background-color: {colors['accent_hover']};
                color: {colors['text']};
                border: 1px solid {colors['accent']};
            }}
        """
        
        # Стиль для невыбранной кнопки
        unselected_button_style = f"""
            QPushButton {{
                background-color: {colors['background_secondary']};
                color: {colors['text_secondary']};
                border: 1px solid {colors['border']};
                border-radius: 4px;
                padding: 4px 8px;
                font-size: 11px;
                min-width: 60px;
            }}
            QPushButton:hover {{
                background-color: {colors['background_hover']};
                color: {colors['text']};
                border: 1px solid {colors['border_hover']};
            }}
        """
        
        # Применяем стили в зависимости от выбранной кнопки
        if self.standard_mode_btn.isChecked():
            self.standard_mode_btn.setStyleSheet(selected_button_style)
            self.preview_mode_btn.setStyleSheet(unselected_button_style)
        elif self.preview_mode_btn.isChecked():
            self.preview_mode_btn.setStyleSheet(selected_button_style)
            self.standard_mode_btn.setStyleSheet(unselected_button_style)
        else:
            self.standard_mode_btn.setStyleSheet(unselected_button_style)
            self.preview_mode_btn.setStyleSheet(unselected_button_style)
        
        # Принудительно обновляем стили
        self.standard_mode_btn.style().unpolish(self.standard_mode_btn)
        self.standard_mode_btn.style().polish(self.standard_mode_btn)
        self.preview_mode_btn.style().unpolish(self.preview_mode_btn)
        self.preview_mode_btn.style().polish(self.preview_mode_btn)
        
        # Обновляем стили через update()
        self.standard_mode_btn.update()
        self.preview_mode_btn.update()
        
        # Принудительно обновляем состояние кнопок
        self.standard_mode_btn.setChecked(self.standard_mode_btn.isChecked())
        self.preview_mode_btn.setChecked(self.preview_mode_btn.isChecked())
        
        # Принудительно обрабатываем события для обновления стилей
        from PyQt6.QtWidgets import QApplication
        QApplication.processEvents()
        
        # Дополнительное принудительное обновление
        self.standard_mode_btn.repaint()
        self.preview_mode_btn.repaint()
        
        # Применяем стиль для стандартного режима
        self.standard_drop.setStyleSheet(styles["folder_drop"])
        self.preview_drop.setStyleSheet(styles["folder_drop"])
        
        # Также применяем начальный стиль для обеспечения правильного отображения границ
        self.apply_initial_style()
    
    def set_preview_offset(self, value):
        self._preview_offset = value  # Используем защищенную переменную
        self.update_preview()

    def get_preview_offset(self):
        return self._preview_offset  # Возвращаем защищенную переменную

    preview_offset = pyqtProperty(float, get_preview_offset, set_preview_offset)

    def on_mode_changed(self, button):
        colors = theme_manager.get_theme_colors()
        
        # Стиль для выбранной кнопки
        selected_button_style = f"""
            QPushButton {{
                background-color: {colors['accent']};
                color: {colors['text']};
                border: 1px solid {colors['accent']};
                border-radius: 4px;
                padding: 4px 8px;
                font-size: 11px;
                min-width: 60px;
            }}
            QPushButton:hover {{
                background-color: {colors['accent_hover']};
                color: {colors['text']};
                border: 1px solid {colors['accent']};
            }}
        """
        
        # Стиль для невыбранной кнопки
        unselected_button_style = f"""
            QPushButton {{
                background-color: {colors['background_secondary']};
                color: {colors['text_secondary']};
                border: 1px solid {colors['border']};
                border-radius: 4px;
                padding: 4px 8px;
                font-size: 11px;
                min-width: 60px;
            }}
            QPushButton:hover {{
                background-color: {colors['background_hover']};
                color: {colors['text']};
                border: 1px solid {colors['border_hover']};
            }}
        """
        
        # Применяем стили в зависимости от выбранной кнопки
        if button == self.standard_mode_btn:
            self.standard_mode_btn.setStyleSheet(selected_button_style)
            self.preview_mode_btn.setStyleSheet(unselected_button_style)
        else:
            self.standard_mode_btn.setStyleSheet(unselected_button_style)
            self.preview_mode_btn.setStyleSheet(selected_button_style)
        
        # Принудительно обновляем стили
        self.standard_mode_btn.style().unpolish(self.standard_mode_btn)
        self.standard_mode_btn.style().polish(self.standard_mode_btn)
        self.preview_mode_btn.style().unpolish(self.preview_mode_btn)
        self.preview_mode_btn.style().polish(self.preview_mode_btn)
        
        # Обновляем стили через update()
        self.standard_mode_btn.update()
        self.preview_mode_btn.update()
        
        # Принудительно обновляем состояние кнопок
        self.standard_mode_btn.setChecked(self.standard_mode_btn.isChecked())
        self.preview_mode_btn.setChecked(self.preview_mode_btn.isChecked())
        
        # Принудительно обрабатываем события для обновления стилей
        from PyQt6.QtWidgets import QApplication
        QApplication.processEvents()
        
        if button == self.standard_mode_btn:
            # Останавливаем анимацию перед сменой режима
            self.preview_timer.stop()
            self.preview_animation.stop()
            self.standard_drop.show()
            self.preview_drop.hide()
            # Применяем стиль для стандартного режима
            self.apply_initial_style()
            self.mode_changed.emit(False)
        else:
            self.standard_drop.hide()
            self.preview_drop.show()
            self.preview_drop.setStyleSheet(f"""
                QLabel {{
                    background-color: transparent;
                    border: 2px dashed {colors['border']};
                    border-radius: 8px;
                    padding: 0px;
                    color: {colors['text_secondary']};
                }}
                QLabel:hover {{
                    border-color: {colors['border_hover']};
                }}
            """)
            if self.preview_images:
                # Сбрасываем состояние анимации
                self._preview_offset = 0
                self.update_preview_image()  # Обновляем превью перед показом
                self.preview_timer.start()
            self.mode_changed.emit(True)

    def set_folder(self, folder_path):
        colors = theme_manager.get_theme_colors()
        if folder_path:
            self.standard_drop.setText(os.path.basename(folder_path))
            self.standard_drop.setToolTip(folder_path)
            self.standard_drop.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.standard_drop.setStyleSheet(f"""
                QLabel {{
                    background-color: transparent;
                    border: 2px dashed {colors['border']};
                    border-radius: 8px;
                    padding: 10px 8px;
                    color: {colors['text']};
                    font-size: 12px;
                }}
                QLabel:hover {{
                    border-color: {colors['border_hover']};
                    color: {colors['text']};
                }}
            """)
            
            # Очищаем предыдущее состояние перед загрузкой
            self.preview_images.clear()
            self.current_preview_index = 0
            self.preview_drop.clear()
            
            # Загружаем изображения для предпросмотра
            self.load_preview_images(folder_path)
        else:
            self.clear_folder()

    def clear_folder(self):
        colors = theme_manager.get_theme_colors()
        self.standard_drop.setText(tr("Click the button below or drag a folder here"))
        self.standard_drop.setToolTip("")
        self.standard_drop.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.standard_drop.setStyleSheet(f"""
            QLabel {{
                background-color: transparent;
                border: 2px dashed {colors['border']};
                border-radius: 8px;
                padding: 10px 8px;
                color: {colors['text_secondary']};
                font-size: 12px;
            }}
            QLabel:hover {{
                border-color: {colors['border_hover']};
                color: {colors['text']};
            }}
        """)
        self.preview_images.clear()
        self.current_preview_index = 0
        
        # Очищаем режим перетащенных изображений
        self.clear_dropped_images_mode()
        
        self.preview_timer.stop()
        self.preview_animation.stop()
        self.preview_drop.clear()

    def on_click(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            folder = QFileDialog.getExistingDirectory(self, tr("Select folder"), os.getcwd())
            if folder:
                self.folder_dropped.emit(folder)

    def load_preview_images(self, folder_path):
        self.preview_images.clear()
        self.current_preview_index = 0
        
        supported_extensions = (".png", ".jpg", ".jpeg", ".bmp", ".gif", ".webp")
        
        try:
            for root, _, files in os.walk(folder_path):
                for file in files:
                    if file.lower().endswith(supported_extensions):
                        self.preview_images.append(os.path.join(root, file))
                        
            if self.preview_images and self.preview_mode_btn.isChecked():
                # Ограничиваем количество превью для лучшей производительности
                if len(self.preview_images) > 15:  # Уменьшаем до 15 для лучшей производительности
                    self.preview_images = random.sample(self.preview_images, 15)
                else:
                    random.shuffle(self.preview_images)
                
                # Обновляем UI для отображения превью
                colors = theme_manager.get_theme_colors()
                self.preview_drop.setStyleSheet(f"""
                    QLabel {{
                        background-color: transparent;
                        border: 2px dashed {colors['border']};
                        border-radius: 8px;
                        padding: 0px;
                        color: {colors['text_secondary']};
                    }}
                """)
                
                # Запускаем отображение превью
                QTimer.singleShot(100, self.update_preview_image)
                self.preview_timer.start()
        except Exception as e:
            logging.error(f"Error loading preview images: {e}")

    def update_preview_image(self):
        if not self.preview_images:
            return
            
        # Выбираем случайное изображение вместо следующего по порядку
        self.current_preview_index = random.randint(0, len(self.preview_images) - 1)
        image_path = self.preview_images[self.current_preview_index]
        
        try:
            # Проверяем кэш превью
            if image_path in self.preview_cache:
                self.current_preview_pixmap = self.preview_cache[image_path]
            else:
                pixmap = QPixmap(image_path)
                if not pixmap.isNull():
                    # Масштабируем изображение точно по ширине виджета (оптимизированное масштабирование)
                    target_width = self.preview_drop.width()
                    if pixmap.width() > target_width:
                        scaled_pixmap = pixmap.scaledToWidth(
                            target_width,
                            Qt.TransformationMode.SmoothTransformation
                        )
                    else:
                        scaled_pixmap = pixmap  # Используем оригинал если он меньше
                    
                    # Кэшируем превью
                    if len(self.preview_cache) >= self.max_preview_cache:
                        # Удаляем самый старый элемент
                        oldest_key = next(iter(self.preview_cache))
                        del self.preview_cache[oldest_key]
                    self.preview_cache[image_path] = scaled_pixmap
                    self.current_preview_pixmap = scaled_pixmap
                
                self.preview_offset = 0
                self.preview_direction *= -1  # Меняем направление анимации
                self.update_preview()  # Сразу обновляем превью
                self.start_preview_animation()
        except Exception as e:
            logging.error(f"Error loading preview: {e}")
            # Пробуем следующее изображение при ошибке
            self.preview_images.remove(image_path) if image_path in self.preview_images else None
            if self.preview_images:
                QTimer.singleShot(100, self.update_preview_image)

    def start_preview_animation(self):
        if not self.current_preview_pixmap:
            return
            
        self.preview_animation.stop()
        
        # Вычисляем максимальное смещение
        max_offset = max(0, self.current_preview_pixmap.height() - self.preview_drop.height())
        
        if self.preview_direction > 0:
            # Анимация снизу вверх
            self.preview_animation.setStartValue(0)
            self.preview_animation.setEndValue(-max_offset)
        else:
            # Анимация сверху вниз
            self.preview_animation.setStartValue(-max_offset)
            self.preview_animation.setEndValue(0)
            
        self.preview_animation.start()

    def update_preview(self):
        if not self.current_preview_pixmap or not self.preview_images or not self.preview_drop.isVisible():
            return
            
        # Получаем размеры
        width = self.preview_drop.width()
        height = self.preview_drop.height()
        
        # Создаем временный QPixmap для рисования содержимого (без рамки)
        content_pixmap = QPixmap(width, height)
        content_pixmap.fill(Qt.GlobalColor.transparent)
        
        # Начинаем рисование на временном pixmap
        painter = QPainter(content_pixmap)
        painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform)
        
        # Рисуем изображение со смещением
        pixmap_height = self.current_preview_pixmap.height()
        offset = int(self._preview_offset)
        painter.drawPixmap(0, offset, self.current_preview_pixmap)
        
        # Обработка циклического отображения
        if offset > 0 and pixmap_height < height + offset:
            painter.drawPixmap(0, offset - pixmap_height, self.current_preview_pixmap)
        if offset < 0 and abs(offset) > height - pixmap_height:
            painter.drawPixmap(0, offset + pixmap_height, self.current_preview_pixmap)
        
        painter.end()
        
        # Создаем финальный QPixmap
        final_pixmap = QPixmap(width, height)
        final_pixmap.fill(Qt.GlobalColor.transparent)
        
        # Рисуем содержимое, обрезая по скруглённой рамке и рисуя рамку цветом темы
        final_painter = QPainter(final_pixmap)
        final_painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform, True)
        final_painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        
        # Клип по скруглённому прямоугольнику
        path = QPainterPath()
        path.addRoundedRect(1, 1, width - 2, height - 2, 8, 8)
        final_painter.setClipPath(path)
        final_painter.drawPixmap(0, 0, content_pixmap)
        
        # Цвет рамки из темы (поддержка rgba(...))
        colors = theme_manager.get_theme_colors()
        border_value = colors.get('border', '#FFFFFF')
        qcolor = QColor('#FFFFFF')
        if isinstance(border_value, str):
            if border_value.startswith('rgba'):
                try:
                    inner = border_value[5:-1]
                    parts = [int(p.strip()) for p in inner.split(',')]
                    if len(parts) == 4:
                        qcolor = QColor(parts[0], parts[1], parts[2], parts[3])
                except Exception:
                    qcolor = QColor('#FFFFFF')
            else:
                qcolor = QColor(border_value)
        
        pen = QPen(qcolor)
        pen.setWidth(2)
        # Синхронизируем штрих с QSS "2px dashed": космический перо, плоские концы, равные штрихи/пробелы
        pen.setCosmetic(True)
        pen.setCapStyle(Qt.PenCapStyle.FlatCap)
        pen.setJoinStyle(Qt.PenJoinStyle.MiterJoin)
        pen.setStyle(Qt.PenStyle.CustomDashLine)
        pen.setDashPattern([4.0, 4.0])  # длина штриха/пробела в px
        pen.setDashOffset(0)
        final_painter.setClipping(False)
        final_painter.setPen(pen)
        final_painter.setBrush(Qt.BrushStyle.NoBrush)
        final_painter.drawRoundedRect(1, 1, width - 2, height - 2, 8, 8)
        final_painter.end()
        
        # Устанавливаем финальный pixmap
        self.preview_drop.setPixmap(final_pixmap)

    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls():
            for url in event.mimeData().urls():
                if os.path.isdir(url.toLocalFile()):
                    event.acceptProposedAction()
                    return
        event.ignore()

    def dropEvent(self, event):
        if event.mimeData().hasUrls():
            for url in event.mimeData().urls():
                folder_path = url.toLocalFile()
                if os.path.isdir(folder_path):
                    self.folder_dropped.emit(folder_path)
                    break
        event.acceptProposedAction()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        if self.current_preview_pixmap and self.preview_drop.isVisible():
            # Перемасштабируем текущее изображение при изменении размера
            scaled_pixmap = self.current_preview_pixmap.scaledToWidth(
                self.preview_drop.width(),
                Qt.TransformationMode.SmoothTransformation
            )
            self.current_preview_pixmap = scaled_pixmap
            self.update_preview()

    def closeEvent(self, event):
        # Останавливаем все анимации перед закрытием
        if hasattr(self, 'preview_timer'):
            self.preview_timer.stop()
        if hasattr(self, 'preview_animation'):
            self.preview_animation.stop()
        super().closeEvent(event)

    def hideEvent(self, event):
        # Stop animations when the widget is hidden
        self.preview_timer.stop()
        self.preview_animation.stop()
        super().hideEvent(event)

class ValueAdjuster(QWidget):
    valueChanged = pyqtSignal(int)
    
    def __init__(self, label, min_value, max_value, step, default_value, parent=None):
        super().__init__(parent)
        
        # Основной layout
        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(2)

        # Верхняя строка с меткой, значением и чекбоксом
        top_layout = QHBoxLayout()
        top_layout.setSpacing(4)

        # Метка без двоеточия
        self.label = QLabel(label)
        self.label.setStyleSheet(f"color: {theme_manager.get_theme_colors()['text']}; font-size: 13px;")
        self.label.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        self.label.setWordWrap(True)
        self.label.setMinimumWidth(200)  # Устанавливаем минимальную ширину
        top_layout.addWidget(self.label)

        # Значение (SpinBox вместо Label)
        self.value_spinbox = QSpinBox()
        self.value_spinbox.setMinimum(min_value)
        self.value_spinbox.setMaximum(max_value)
        self.value_spinbox.setValue(default_value)
        self.value_spinbox.setSingleStep(step)
        self.value_spinbox.setStyleSheet(f"""
            QSpinBox {{
                color: {theme_manager.get_theme_colors()['text']};
                background-color: {theme_manager.get_theme_colors()['background_secondary']};
                border: 1px solid {theme_manager.get_theme_colors()['border']};
                border-radius: 4px;
                padding: 1px 1px 1px 4px;
                min-width: 60px;
                max-width: 60px;
                font-size: 13px;
            }}
            QSpinBox:hover {{
                background-color: {theme_manager.get_theme_colors()['background_hover']};
                border: 1px solid {theme_manager.get_theme_colors()['border_hover']};
            }}
            QSpinBox:focus {{
                border: 1px solid {theme_manager.get_theme_colors()['border_focus']};
                background-color: {theme_manager.get_theme_colors()['background_hover']};
            }}
            QSpinBox::up-button, QSpinBox::down-button {{
                width: 0;
                border: none;
            }}
        """)
        self.value_spinbox.setAlignment(Qt.AlignmentFlag.AlignCenter)
        top_layout.addWidget(self.value_spinbox)

        # Чекбокс бесконечности - только символ ∞ без слова "unlimited"
        self.unlimited_checkbox = QCheckBox("∞")
        self.unlimited_checkbox.setToolTip(tr("Unlimited"))
        self.unlimited_checkbox.setStyleSheet(f"""
            QCheckBox {{
                color: {theme_manager.get_theme_colors()['text']};
                font-size: 14px;
                spacing: 4px;
            }}
            QCheckBox::indicator {{
                width: 16px;
                height: 16px;
                border-radius: 4px;
                border: 1px solid {theme_manager.get_theme_colors()['border']};
                background: {theme_manager.get_theme_colors()['background_secondary']};
            }}
            QCheckBox::indicator:hover {{
                border: 1px solid {theme_manager.get_theme_colors()['border_hover']};
                background: {theme_manager.get_theme_colors()['background_hover']};
            }}
            QCheckBox::indicator:checked {{
                background: {theme_manager.get_theme_colors()['background_checked']};
                border: 1px solid {theme_manager.get_theme_colors()['border_hover']};
            }}
        """)
        top_layout.addWidget(self.unlimited_checkbox)

        main_layout.addLayout(top_layout)

        # Слайдер
        self.slider = QSlider(Qt.Orientation.Horizontal)
        self.slider.setMinimum(min_value)
        self.slider.setMaximum(max_value)
        self.slider.setValue(default_value)
        self.slider.setFixedHeight(24)  # Уменьшаем высоту слайдера
        
        # Устанавливаем метки на слайдер
        self.slider.setTickPosition(QSlider.TickPosition.TicksBelow)
        
        # Определяем интервалы меток в зависимости от типа слайдера
        if label == tr("Volume:"):
            self.slider.setTickInterval(10)
        elif label == tr("Display time (sec):"):
            self.slider.setTickInterval(60)  # Каждые 60 секунд
        elif label == tr("Number of images:"):
            self.slider.setTickInterval(50)  # Каждые 50 изображений
        elif label == tr("Duration (min):"):
            self.slider.setTickInterval(10)  # Каждые 10 минут
        
        # Добавляем подписи значений
        self.min_label = QLabel(str(min_value))
        self.min_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
        self.max_label = QLabel(str(max_value))
        self.max_label.setAlignment(Qt.AlignmentFlag.AlignRight)

        # Получаем цвета из текущей темы
        colors = theme_manager.get_theme_colors()
        labels_style = f"color: {colors['text_secondary']}; font-size: 10px; margin-top: -4px; font-family: 'Segoe UI', Arial;"
        self.min_label.setStyleSheet(labels_style)
        self.max_label.setStyleSheet(labels_style)

        # Определяем специальные подписи в зависимости от типа слайдера
        if label == tr("Volume:"):
            self.min_label.setText("0%")
            self.max_label.setText("100%")
        elif label == tr("Display time (sec):"):
            self.min_label.setText("1s")
            self.max_label.setText("900s")
        elif label == tr("Number of images:"):
            self.min_label.setText("1")
            self.max_label.setText("900")
        elif label == tr("Duration (min):"):
            self.min_label.setText("1")
            self.max_label.setText("60")

        # Создаем контейнер для подписей
        labels_layout = QHBoxLayout()
        labels_layout.setContentsMargins(8, 0, 8, 0)
        labels_layout.addWidget(self.min_label)
        labels_layout.addStretch()
        labels_layout.addWidget(self.max_label)
        
        self.slider.setPageStep(step)
        self.slider.setStyleSheet(theme_manager.get_theme_styles()["slider"])
        main_layout.addWidget(self.slider)
        main_layout.addLayout(labels_layout)

        # Подключаем сигналы
        self.slider.valueChanged.connect(self.update_value_from_slider)
        self.value_spinbox.valueChanged.connect(self.update_value_from_spinbox)
        self.unlimited_checkbox.toggled.connect(self.on_unlimited_toggled)

    def update_value_from_slider(self, value):
        if not self.unlimited_checkbox.isChecked():
            self.value_spinbox.setValue(value)
            self.valueChanged.emit(value)

    def update_value_from_spinbox(self, value):
        if not self.unlimited_checkbox.isChecked():
            self.slider.setValue(value)
            self.valueChanged.emit(value)

    def on_unlimited_toggled(self, checked):
        self.slider.setEnabled(not checked)
        self.value_spinbox.setEnabled(not checked)
        if checked:
            self.value_spinbox.setSpecialValueText("∞")
            self.valueChanged.emit(999999)
        else:
            self.value_spinbox.setSpecialValueText("")
            self.value_spinbox.setValue(self.slider.value())
            self.valueChanged.emit(self.slider.value())

    def value(self):
        return 999999 if self.unlimited_checkbox.isChecked() else self.value_spinbox.value()

    def setValue(self, value):
        if value == 999999:
            self.unlimited_checkbox.setChecked(True)
        else:
            self.unlimited_checkbox.setChecked(False)
            self.value_spinbox.setValue(value)
            self.slider.setValue(value)

    def setUnlimited(self, unlimited):
        self.unlimited_checkbox.setChecked(unlimited)

    def apply_theme(self):
        """Обновляет стили элементов в соответствии с текущей темой"""
        styles = theme_manager.get_theme_styles()
        colors = theme_manager.get_theme_colors()
        
        # Обновляем стиль метки
        self.label.setStyleSheet(f"color: {colors['text']};")
        
        # Обновляем стиль SpinBox
        self.value_spinbox.setStyleSheet(f"""
            QSpinBox {{
                color: {colors['text']};
                background-color: {colors['background_secondary']};
                border: 1px solid {colors['border']};
                border-radius: 4px;
                padding: 1px 1px 1px 4px;
                min-width: 60px;
                max-width: 60px;
                font-size: 13px;
            }}
            QSpinBox:hover {{
                background-color: {colors['background_hover']};
                border: 1px solid {colors['border_hover']};
            }}
            QSpinBox:focus {{
                border: 1px solid {colors['border_focus']};
                background-color: {colors['background_hover']};
            }}
            QSpinBox::up-button, QSpinBox::down-button {{
                width: 0;
                border: none;
            }}
        """)
        
        # Обновляем стиль чекбокса
        self.unlimited_checkbox.setStyleSheet(f"""
            QCheckBox {{
                color: {colors['text']};
                font-size: 14px;
                spacing: 4px;
            }}
            QCheckBox::indicator {{
                width: 16px;
                height: 16px;
                border-radius: 4px;
                border: 1px solid {colors['border']};
                background: {colors['background_secondary']};
            }}
            QCheckBox::indicator:hover {{
                border: 1px solid {colors['border_hover']};
                background: {colors['background_hover']};
            }}
            QCheckBox::indicator:checked {{
                background: {colors['background_checked']};
                border: 1px solid {colors['border_hover']};
            }}
        """)
        
        # Обновляем стиль подписей min/max
        if hasattr(self, 'min_label') and hasattr(self, 'max_label'):
            labels_style = f"color: {colors['text_secondary']}; font-size: 10px; margin-top: -4px; font-family: 'Segoe UI', Arial;"
            self.min_label.setStyleSheet(labels_style)
            self.max_label.setStyleSheet(labels_style)
        
        # Обновляем стиль слайдера
        self.slider.setStyleSheet(styles["slider"])

class TimerPositionSelector(QWidget):
    positionChanged = pyqtSignal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        
        # Создаем layout
        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)

        # Добавляем заголовок без двоеточия
        self.label = QLabel(tr("Timer position"))
        self.label.setProperty('isHeader', True)
        self.label.original_text = "Timer position"
        layout.addWidget(self.label)
        layout.addStretch()

        # Создаем QComboBox
        self.combo = QComboBox()
        self.combo.addItem(tr("Left"), "left")
        self.combo.addItem(tr("Center"), "center")
        self.combo.addItem(tr("Right"), "right")
        
        # Устанавливаем фиксированную ширину для комбобокса
        self.combo.setFixedWidth(140)
        
        # Стиль для комбобокса
        colors = theme_manager.get_theme_colors()
        self.combo.setStyleSheet(f"""
            QComboBox {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                border: 1px solid {colors['border']};
                border-radius: 4px;
                padding: 4px 24px 4px 8px;
            }}
            QComboBox:hover {{
                background-color: {colors['background_hover']};
                color: {colors['text']};
                border: 1px solid {colors['border_hover']};
            }}
            QComboBox::drop-down {{
                border: none;
                width: 24px;
            }}
            QComboBox::down-arrow {{
                image: url(resources/down_arrow.png);
                width: 12px;
                height: 12px;
            }}
            QComboBox QAbstractItemView {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                selection-background-color: {colors['accent']};
                selection-color: {colors['text']};
                border: 1px solid {colors['border']};
                padding: 4px 0px;
            }}
        """)

        # Добавляем комбобокс в layout
        layout.addWidget(self.combo)

        # Подключаем сигнал
        self.combo.currentIndexChanged.connect(self.on_position_changed)

    def retranslate_ui(self):
        """Обновляет все тексты при смене языка"""
        self.label.setText(tr("Timer position"))
        
        current_position = self.combo.currentData()
        
        # Блокируем сигнал перед обновлением
        self.combo.blockSignals(True)
        
        self.combo.clear()
        self.combo.addItem(tr("Left"), "left")
        self.combo.addItem(tr("Center"), "center")
        self.combo.addItem(tr("Right"), "right")
        
        # Восстанавливаем выбранную позицию
        for i in range(self.combo.count()):
            if self.combo.itemData(i) == current_position:
                self.combo.setCurrentIndex(i)
                break
                
        # Разблокируем сигнал после обновления
        self.combo.blockSignals(False)

    def on_position_changed(self, index):
        position = self.combo.itemData(index)
        self.positionChanged.emit(position)

    def setPosition(self, position):
        index = self.combo.findData(position)
        if index >= 0:
            self.combo.setCurrentIndex(index)

    def currentPosition(self):
        return self.combo.currentData()

# Основное окно показа изображений (интерфейс 2)
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("GestArt")
        self.resize(1200, 800)
        self.setMouseTracking(True)
        self.setStatusBar(None)
        
        # Устанавливаем иконку программы
        app_icon = QIcon(get_resource_path("resources/gestart-logo.png"))
        self.setWindowIcon(app_icon)
        
        # Инициализируем менеджер конфигурации
        self.config_manager = ConfigManager()
        
        # Расширенный список поддерживаемых форматов
        self.supported_extensions = (
            ".png", ".jpg", ".jpeg", ".bmp", ".gif", ".webp", 
            ".tiff", ".tif", ".ico", ".svg", ".heic", ".heif"
        )
        
        # Загружаем настройки
        self.settings = self.config_manager.load_settings()
        
        # Инициализируем историю просмотров
        if self.settings.get("save_history", True):
            self.displayed_history = self.config_manager.load_history()
        else:
            self.displayed_history = []
        
        # Применяем тему
        theme_manager.set_theme(self.settings.get("theme", "dark"))
        
        # Применяем язык
        translation_manager.set_locale(self.settings.get("language", "en"))
        
        # Создаем таймер для проверки системной темы
        self.system_theme_check_timer = QTimer(self)
        self.system_theme_check_timer.setInterval(5000)  # Проверка каждые 5 секунд
        self.system_theme_check_timer.timeout.connect(self.check_system_theme)
        # Запускаем таймер только если выбрана системная тема
        if self.settings.get("theme") == "system":
            self.system_theme_check_timer.start()
        
        # Загружаем статистику просмотров для папок
        self.folder_stats = self.config_manager.load_folder_stats()
        
        # Если есть сохраненная папка, загружаем изображения без запуска сессии
        if self.settings.get("folder"):
            current_folder = os.path.abspath(self.settings["folder"])
            if current_folder not in self.folder_stats:
                self.folder_stats[current_folder] = 0
            self.config_manager.save_folder_stats(self.folder_stats)
            
            # Загружаем изображения из сохраненной папки без запуска сессии
            if os.path.isdir(current_folder):
                self.scanner_thread = FolderScannerThread(current_folder, self.supported_extensions)
                self.scanner_thread.scanned.connect(self.on_images_scanned)
                self.scanner_thread.error.connect(self.on_scanner_error)
                self.scanner_thread.progress.connect(self.on_progress)
                self.scanner_thread.start()
        
        # Создаем центральный виджет
        self.central_widget = QWidget()
        self.central_widget.setMouseTracking(True)
        self.setCentralWidget(self.central_widget)

        # Основной layout для всего окна
        self.main_layout = QVBoxLayout(self.central_widget)
        self.main_layout.setContentsMargins(0, 0, 0, 0)
        self.main_layout.setSpacing(0)

        # Загрузка настроек и истории
        self.settings = self.config_manager.load_settings()
        self.displayed_history = self.config_manager.load_history()
        
        # Создаем контейнер для изображения
        self.image_container = QWidget()
        self.image_container.setLayout(QVBoxLayout())
        self.image_container.layout().setContentsMargins(0, 0, 0, 0)
        self.image_container.layout().setSpacing(0)
        
        # Область для изображения (на весь экран)
        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.image_label.setStyleSheet("background-color: #2d2d30;")
        self.image_label.setMinimumSize(1, 1)  # Устанавливаем минимальный размер
        
        # Создаем layout для image_label для правильного центрирования контента
        image_layout = QVBoxLayout(self.image_label)
        image_layout.setContentsMargins(0, 0, 0, 0)
        image_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        # Добавляем логотип-меню при запуске
        self.menu_logo = QLabel()
        self.menu_logo.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.menu_logo.setStyleSheet("background: transparent;")
        
        # Загружаем оригинальное изображение
        original_pixmap = QPixmap(get_resource_path("resources/gestart-menu.png"))
        
        # Уменьшаем размер на треть (2/3 от оригинального размера)
        scaled_width = int(original_pixmap.width() * 2/3)
        scaled_height = int(original_pixmap.height() * 2/3)
        
        # Масштабируем изображение с сохранением пропорций
        self.menu_logo_pixmap = original_pixmap.scaled(scaled_width, scaled_height, 
                                                    Qt.AspectRatioMode.KeepAspectRatio, 
                                                    Qt.TransformationMode.SmoothTransformation)
        
        # Устанавливаем уменьшенное изображение
        self.menu_logo.setPixmap(self.menu_logo_pixmap)
        
        # Добавляем логотип в layout изображения
        image_layout.addWidget(self.menu_logo)
        
        # Добавляем image_label в контейнер
        self.image_container.layout().addWidget(self.image_label)
        
        # Добавляем контейнер с изображением в основной layout
        self.main_layout.addWidget(self.image_container)

        # Создаем оверлей для всех элементов управления
        self.overlay_widget = QWidget(self.central_widget)
        self.overlay_widget.setMouseTracking(True)
        self.overlay_widget.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, False)
        self.overlay_widget.setGeometry(0, 0, self.width(), self.height())
        self.overlay_widget.setStyleSheet("background: transparent;")
        
        # Layout для оверлея
        overlay_layout = QVBoxLayout(self.overlay_widget)
        overlay_layout.setContentsMargins(10, 10, 10, 10)
        overlay_layout.setSpacing(10)

        # Таймер сверху
        timer_container = QWidget()
        timer_container.setStyleSheet("""
            QWidget {
                background-color: rgba(0, 0, 0, 120);
                border-radius: 4px;
                padding: 2px 8px;
            }
        """)
        timer_layout = QHBoxLayout(timer_container)
        timer_layout.setContentsMargins(0, 0, 0, 0)
        
        self.timer_label = QLabel()
        self.timer_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.timer_label.setStyleSheet("""
            font-weight: bold;
            color: white;
            padding: 2px;
        """)
        timer_layout.addWidget(self.timer_label)
        overlay_layout.addWidget(timer_container, 0, Qt.AlignmentFlag.AlignHCenter | Qt.AlignmentFlag.AlignTop)
        
        # Добавляем индикатор прогресса
        self.progress_label = QLabel()
        self.progress_label.setStyleSheet("""
            QLabel {
                background-color: rgba(0, 0, 0, 120);
                color: white;
                padding: 5px;
                border-radius: 4px;
            }
        """)
        self.progress_label.hide()
        overlay_layout.addWidget(self.progress_label, 0, Qt.AlignmentFlag.AlignHCenter | Qt.AlignmentFlag.AlignTop)
        
        # Добавляем растягивающийся элемент, чтобы прижать кнопки к низу
        overlay_layout.addStretch(1)

        # Создаем центрирующий контейнер с горизонтальным макетом
        centering_widget = QWidget()
        centering_widget.setContentsMargins(0, 0, 0, 0)
        centering_layout = QHBoxLayout(centering_widget)
        centering_layout.setContentsMargins(0, 0, 0, 0)
        centering_layout.setSpacing(0)
        
        # Добавляем растягивающиеся элементы с одинаковым коэффициентом для идеального центрирования
        centering_layout.addStretch(1)
        
        # Создаем контейнер для кнопок с прозрачным фоном
        buttons_container = QWidget()
        # Устанавливаем политику размера, чтобы контейнер занимал только необходимое пространство
        buttons_container.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        buttons_container.setStyleSheet(theme_manager.get_theme_styles()["buttons_container"])
        
        # Настраиваем макет контейнера кнопок
        buttons_layout = QVBoxLayout(buttons_container)
        buttons_layout.setContentsMargins(10, 10, 10, 10)
        buttons_layout.setSpacing(10)
        buttons_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        # Добавляем контейнер кнопок в центрирующий слой
        centering_layout.addWidget(buttons_container)
        
        # Добавляем еще один растягивающийся элемент для баланса
        centering_layout.addStretch(1)
        
        # Добавляем центрирующий виджет в основной слой
        overlay_layout.addWidget(centering_widget, 0, Qt.AlignmentFlag.AlignHCenter | Qt.AlignmentFlag.AlignBottom)

        # Первая строка - кнопки навигации
        nav_layout = QHBoxLayout()
        nav_layout.setSpacing(10)  # Увеличиваем отступ для единообразия
        nav_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        self.prev_button = QPushButton()
        self.prev_button.setIcon(create_themed_icon("resources/past.png"))
        self.prev_button.setIconSize(QSize(42, 42))
        self.prev_button.setToolTip(tr("Previous image") + " (←)")
        self.prev_button.clicked.connect(self.show_previous_image)
        self.prev_button.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        nav_layout.addWidget(self.prev_button)

        self.pause_button = QPushButton()
        self.pause_button.setIcon(create_themed_icon("resources/pause0.png"))
        self.pause_button.setIconSize(QSize(28, 28))
        self.pause_button.setToolTip(tr("Pause") + " (Space)")
        self.pause_button.clicked.connect(self.toggle_pause)
        self.pause_button.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        nav_layout.addWidget(self.pause_button)

        self.skip_button = QPushButton()
        self.skip_button.setIcon(create_themed_icon("resources/skip0.png"))
        self.skip_button.setIconSize(QSize(28, 28))
        self.skip_button.setToolTip(tr("Skip") + " (S)")
        self.skip_button.clicked.connect(self.skip_image)
        self.skip_button.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        nav_layout.addWidget(self.skip_button)

        self.next_button = QPushButton()
        self.next_button.setIcon(create_themed_icon("resources/next.png"))
        self.next_button.setIconSize(QSize(42, 42))
        self.next_button.setToolTip(tr("Next image") + " (→)")
        self.next_button.clicked.connect(self.show_next_image)
        self.next_button.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        nav_layout.addWidget(self.next_button)

        buttons_layout.addLayout(nav_layout)

        # Вторая строка - остальные кнопки без группировки
        tools_layout = QHBoxLayout()
        tools_layout.setSpacing(2)  # Минимальный отступ между кнопками инструментов
        tools_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.open_file_button = QPushButton()
        self.open_file_button.setIcon(create_themed_icon("resources/file0.png"))
        self.open_file_button.setIconSize(QSize(24, 24))
        self.open_file_button.setToolTip(tr("Open file") + " (O)")
        self.open_file_button.clicked.connect(self.open_current_file)
        tools_layout.addWidget(self.open_file_button)

        # Добавляем кнопку копирования
        self.copy_image_button = QPushButton()
        self.copy_image_button.setIcon(create_themed_icon("resources/copy0.png"))
        self.copy_image_button.setIconSize(QSize(24, 24))
        self.copy_image_button.setToolTip(tr("Copy image to clipboard") + " (C)")
        self.copy_image_button.clicked.connect(self.copy_image_to_clipboard)
        tools_layout.addWidget(self.copy_image_button)
        
        # Добавляем кнопку удаления
        self.delete_button = QPushButton()
        self.delete_button.setIcon(create_themed_icon("resources/delete0.png"))
        self.delete_button.setIconSize(QSize(24, 24))
        self.delete_button.setToolTip(tr("Move to trash") + " (Delete)")
        self.delete_button.clicked.connect(self.delete_current_file)
        tools_layout.addWidget(self.delete_button)
        
        # Добавляем кнопку сетки
        self.grid_button = QPushButton()
        self.grid_button.setIcon(create_themed_icon("resources/grid0.png"))
        self.grid_button.setIconSize(QSize(24, 24))
        self.grid_button.setToolTip(tr("Toggle grid") + " (G)")
        self.grid_button.setCheckable(True)
        self.grid_button.clicked.connect(self.toggle_grid)
        tools_layout.addWidget(self.grid_button)

        self.bw_button = QPushButton()
        self.bw_button.setIcon(create_themed_icon("resources/bwfilter0.png"))
        self.bw_button.setIconSize(QSize(24, 24))
        self.bw_button.setToolTip(tr("B/W filter") + " (B)")
        self.bw_button.clicked.connect(self.apply_bw_filter)
        tools_layout.addWidget(self.bw_button)

        self.flip_v_button = QPushButton()
        self.flip_v_button.setIcon(create_themed_icon("resources/flipv.png"))
        self.flip_v_button.setIconSize(QSize(24, 24))
        self.flip_v_button.setToolTip(tr("Flip vertically") + " (V)")
        self.flip_v_button.clicked.connect(self.flip_vertical)
        tools_layout.addWidget(self.flip_v_button)

        self.flip_h_button = QPushButton()
        self.flip_h_button.setIcon(create_themed_icon("resources/fliph.png"))
        self.flip_h_button.setIconSize(QSize(24, 24))
        self.flip_h_button.setToolTip(tr("Flip horizontally") + " (H)")
        self.flip_h_button.clicked.connect(self.flip_horizontal)
        tools_layout.addWidget(self.flip_h_button)

        self.rotate_button = QPushButton()
        self.rotate_button.setIcon(create_themed_icon("resources/rotate.png"))
        self.rotate_button.setIconSize(QSize(24, 24))
        self.rotate_button.setToolTip(tr("Rotate 90°") + " (R)")
        self.rotate_button.clicked.connect(self.rotate_90)
        tools_layout.addWidget(self.rotate_button)

        self.restore_button = QPushButton()
        self.restore_button.setIcon(create_themed_icon("resources/reset.png"))
        self.restore_button.setIconSize(QSize(24, 24))
        self.restore_button.setToolTip(tr("Reset image") + " (Backspace)")
        self.restore_button.clicked.connect(self.restore_original)
        tools_layout.addWidget(self.restore_button)

        self.show_timer_button = QPushButton()
        self.show_timer_button.setIcon(create_themed_icon("resources/timer0.png"))
        self.show_timer_button.setIconSize(QSize(24, 24))
        self.show_timer_button.setToolTip(tr("Show/Hide timer") + " (T)")
        self.show_timer_button.setCheckable(True)
        self.show_timer_button.setChecked(self.settings.get("show_timer", True))
        self.show_timer_button.clicked.connect(self.toggle_show_timer)
        tools_layout.addWidget(self.show_timer_button)

        # Устанавливаем начальную иконку в зависимости от состояния
        if self.show_timer_button.isChecked():
            self.show_timer_button.setIcon(create_themed_icon("resources/timer1.png"))
        else:
            self.show_timer_button.setIcon(create_themed_icon("resources/timer0.png"))

        self.always_on_top_button = QPushButton()
        self.always_on_top_button.setIcon(create_themed_icon("resources/pin0.png"))
        self.always_on_top_button.setIconSize(QSize(24, 24))
        self.always_on_top_button.setToolTip(tr("Always on top") + " (A)")
        self.always_on_top_button.setCheckable(True)
        self.always_on_top_button.clicked.connect(self.toggle_always_on_top)
        tools_layout.addWidget(self.always_on_top_button)

        self.settings_button = QPushButton()
        self.settings_button.setIcon(create_themed_icon("resources/settings0.png"))
        self.settings_button.setIconSize(QSize(24, 24))
        self.settings_button.setToolTip(tr("Settings") + " (Ctrl+,)")
        self.settings_button.clicked.connect(self.open_settings)
        self.settings_button.setStyleSheet("""
            QPushButton {
                background-color: transparent;
                border: none;
                padding: 0;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 10);
            }
        """)
        self.settings_button.setFixedSize(36, 36)
        tools_layout.addWidget(self.settings_button)

        buttons_layout.addLayout(tools_layout)

        # Стиль для кнопок навигации
        nav_button_style = """
            QPushButton {
                background-color: rgba(45, 45, 45, 180);
                color: white;
                border: 1px solid rgba(255, 255, 255, 20);
                border-radius: 8px;
                padding: 8px;
                width: 90px;
                height: 36px;
                font-size: 18px;
                font-weight: bold;
                margin: 0 5px;
            }
            QPushButton:hover {
                background-color: rgba(55, 55, 55, 200);
                border: 1px solid rgba(255, 255, 255, 40);
                color: rgba(255, 255, 255, 255);
            }
            QPushButton:pressed {
                background-color: rgba(35, 35, 35, 200);
                border: 1px solid rgba(255, 255, 255, 30);
            }
            QPushButton:checked {
                background-color: rgba(70, 130, 180, 200);
                border: 1px solid rgba(255, 255, 255, 40);
            }
        """
        
        # Стиль для остальных кнопок
        button_style = """
            QPushButton {
                background-color: rgba(45, 45, 45, 180);
                color: white;
                border: 1px solid rgba(255, 255, 255, 20);
                border-radius: 8px;
                padding: 0px;
                width: 36px;
                height: 36px;
                font-size: 16px;
                font-weight: bold;
                margin: 0 1px;
            }
            QPushButton:hover {
                background-color: rgba(55, 55, 55, 200);
                border: 1px solid rgba(255, 255, 255, 40);
                color: rgba(255, 255, 255, 255);
            }
            QPushButton:pressed {
                background-color: rgba(35, 35, 35, 200);
                border: 1px solid rgba(255, 255, 255, 30);
            }
            QPushButton:checked {
                background-color: rgba(70, 130, 180, 200);
                border: 1px solid rgba(255, 255, 255, 40);
            }
        """

        # Применяем стиль к кнопкам навигации
        self.prev_button.setStyleSheet(nav_button_style)
        self.pause_button.setStyleSheet(nav_button_style)
        self.skip_button.setStyleSheet(nav_button_style)
        self.next_button.setStyleSheet(nav_button_style)
        
        # Устанавливаем фиксированные размеры для навигационных кнопок
        for button in [self.prev_button, self.pause_button, self.skip_button, self.next_button]:
            button.setFixedSize(90, 36)

        # Применяем стиль к остальным кнопкам
        for button in [self.open_file_button, self.copy_image_button, self.delete_button,
                       self.grid_button, self.bw_button, self.flip_v_button,
                       self.flip_h_button, self.rotate_button, self.restore_button,
                       self.show_timer_button, self.always_on_top_button]:
            button.setStyleSheet(button_style)
            button.setFixedSize(36, 36)
            
        # Устанавливаем минимальный отступ для инструментальных кнопок
        tools_layout.setSpacing(4)

        # Список всех элементов для управления видимостью
        self.overlay_controls = [buttons_container]

        # Настройки по умолчанию
        self.image_files = []             # список путей к изображениям
        self.accepted_count = 0           # количество показанных изображений
        self.session_images = []          # список показанных изображений (без пропусков)
        self.history_index = -1           # индекс текущего изображения
        self.current_image_path = None
        self.current_pixmap = None
        self.original_pixmap = None
        self.is_paused = False
        self.is_session_completed = False  # Флаг завершения сессии
        self.is_in_break = False  # Флаг режима перерыва

        # Добавляем ограничение на размер кэша (увеличиваем для лучшей производительности)
        self.max_cache_size = 100  # Увеличиваем размер кэша
        self.pixmap_cache = {}
        self.cache_access_order = []  # Для LRU кэширования
        
        # Добавляем счетчик попыток загрузки
        self.load_attempts = 0
        self.max_load_attempts = 3

        # Таймеры
        self.remaining_time = self.settings["display_time"]
        self.countdown_timer = QTimer()
        self.countdown_timer.setInterval(1000)
        self.countdown_timer.timeout.connect(self.update_countdown)

        self.break_timer = QTimer()
        self.break_timer.setSingleShot(True)
        self.break_timer.timeout.connect(self.start_session)

        # Поток сканирования папки
        self.scanner_thread = None

        # Добавляем флаги для глобальных эффектов
        self.is_bw = False
        self.flip_v_active = False
        self.flip_h_active = False
        self.rotation_angle = 0

        # Делаем кнопки эффектов переключаемыми
        self.bw_button.setCheckable(True)
        self.flip_v_button.setCheckable(True)
        self.flip_h_button.setCheckable(True)

        # Добавляем переменные для зума
        self.zoom_factor = 1.0
        self.zoom_center = None  # Центр зума
        self.is_dragging = False  # Флаг для перетаскивания
        self.last_mouse_pos = None  # Последняя позиция мыши
        self.pan_x = 0  # Смещение по X при панорамировании
        self.pan_y = 0  # Смещение по Y при панорамировании
        
        # Убираем отслеживание колеса мыши с image_label
        self.image_label.setMouseTracking(True)

        # Инициализируем видимость таймера в соответствии с настройками
        if not self.settings.get("show_timer", True):
            self.timer_label.hide()

        self.update_timer_label()

        # Добавляем хоткей пробел для продолжения после перерыва
        space_shortcut = QShortcut(QKeySequence("Space"), self)
        space_shortcut.activated.connect(self.handle_space_key)

        # Добавляем горячие клавиши для кнопок
        self.prev_button.setShortcut("Left")
        self.next_button.setShortcut("Right")
        self.skip_button.setShortcut("S")
        self.open_file_button.setShortcut("O")
        self.copy_image_button.setShortcut("C")  # Новый хоткей для копирования
        self.grid_button.setShortcut("G")  # Новый хоткей для сетки
        self.delete_button.setShortcut("Delete")
        self.bw_button.setShortcut("B")
        self.flip_v_button.setShortcut("V")
        self.flip_h_button.setShortcut("H")
        self.rotate_button.setShortcut("R")
        self.restore_button.setShortcut("Backspace")
        self.show_timer_button.setShortcut("T")
        self.always_on_top_button.setShortcut("A")
        self.settings_button.setShortcut("Ctrl+,")

        # Добавляем словарь для отслеживания загрузчиков
        self.image_loaders = {}

        # Добавляем переменную для отслеживания времени перерыва
        self.break_remaining_time = 0

        # Добавляем флаг для отслеживания состояния перерыва
        self.is_in_break = False

        # Добавляем переменные для сетки
        self.show_grid = False
        self.grid_h_lines = 1  # Количество горизонтальных линий (1-4)
        self.grid_v_lines = 1  # Количество вертикальных линий (1-4)
        self.grid_color = Qt.GlobalColor.white  # Цвет сетки
        self.grid_line_width = 1  # Толщина линий сетки

        # Добавляем индикатор зума
        self.zoom_indicator = QLabel(self)
        self.zoom_indicator.setStyleSheet("""
            QLabel {
                background-color: rgba(0, 0, 0, 120);
                color: white;
                padding: 4px 8px;
                border-radius: 4px;
                font-size: 12px;
            }
        """)
        self.zoom_indicator.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.zoom_indicator.hide()
        
        # Таймер для скрытия индикатора зума
        self.zoom_indicator_timer = QTimer(self)
        self.zoom_indicator_timer.setSingleShot(True)
        self.zoom_indicator_timer.timeout.connect(lambda: self.zoom_indicator.hide())

        # Добавляем горячие клавиши для зума
        zoom_in_shortcut = QShortcut(QKeySequence("Ctrl++"), self)
        zoom_in_shortcut.activated.connect(self.zoom_in)
        
        zoom_out_shortcut = QShortcut(QKeySequence("Ctrl+-"), self)
        zoom_out_shortcut.activated.connect(self.zoom_out)
        
        reset_zoom_shortcut = QShortcut(QKeySequence("Ctrl+0"), self)
        reset_zoom_shortcut.activated.connect(self.reset_zoom)

        # Инициализируем переменные для таймера
        self.is_paused = False
        self.is_session_completed = False
        self.is_in_break = False
        self.remaining_time = 0
        self.break_remaining_time = 0
        self.accepted_count = 0
        self.load_attempts = 0
        self.max_load_attempts = 3
        
        # Инициализируем аудиоплеер для звука таймера
        self.timer_sound_player = QMediaPlayer()
        self.timer_audio_output = QAudioOutput()
        self.timer_sound_player.setAudioOutput(self.timer_audio_output)
        self.timer_sound_player.setSource(QUrl.fromLocalFile(get_resource_path("resources/mixkit-game-count.wav")))
        self.timer_sound_playing = False
        
        # Применяем тему
        self.apply_theme()
        
        # Обновляем подсказки с правильными стилями темы
        self.update_tooltips()

        self.load_error_streak = 0 # Counter for consecutive load errors
        self.max_load_error_streak = 5 # Allow 5 consecutive errors before stopping

    # При входе курсора – показываем контролы
    def enterEvent(self, event):
        self.show_controls()
        super().enterEvent(event)

    # При уходе курсора – скрываем контролы, если курсор вне окна
    def leaveEvent(self, event):
        if not self.rect().contains(self.mapFromGlobal(self.cursor().pos())):
            self.hide_controls()
        super().leaveEvent(event)

    def hide_controls(self):
        timer_container = self.timer_label.parent()
        if not self.show_timer_button.isChecked():
            timer_container.hide()
        for widget in self.overlay_controls:
            widget.hide()

    def show_controls(self):
        timer_container = self.timer_label.parent()
        if self.show_timer_button.isChecked():
            timer_container.show()
        for widget in self.overlay_controls:
            widget.show()

    def open_settings(self):
        """Открывает окно настроек"""
        # Останавливаем таймер при открытии настроек
        if hasattr(self, 'countdown_timer') and self.countdown_timer.isActive():
            self.countdown_timer.stop()
            self.is_paused = True
            self.pause_button.setIcon(create_themed_icon("resources/play0.png"))  # Используем обновленную функцию
            self.pause_button.setToolTip(tr("Resume") + " (Space)")
            
        dialog = SettingsDialog(self, self.settings)
        dialog.settings_updated.connect(self.apply_new_settings)
        dialog.exec()
        
        # Снимаем с паузы после закрытия настроек
        if self.is_paused:
            self.is_paused = False
            self.pause_button.setIcon(create_themed_icon("resources/pause0.png"))  # Используем обновленную функцию
            self.pause_button.setToolTip(tr("Pause") + " (Space)")
            if hasattr(self, 'countdown_timer'):
                self.countdown_timer.start()

    def apply_new_settings(self, new_settings):
        """Применяет новые настройки"""
        # Сохраняем старые настройки для сравнения
        old_settings = self.settings.copy()
            
        # Обновляем настройки
        self.settings.update(new_settings)
        self.config_manager.save_settings(self.settings)
        
        # Применяем новый язык, если он изменился
        if new_settings.get("language") != old_settings.get("language"):
            translation_manager.set_locale(new_settings["language"])
            self.retranslate_ui()
            
            # Обновляем тексты в окне настроек, если оно открыто
            for child in self.children():
                if isinstance(child, SettingsDialog):
                    child.retranslate_ui()
                    break
        
        # Применяем новую тему, если она изменилась
        if new_settings.get("theme") != old_settings.get("theme"):
            # Обновляем таймер проверки системной темы
            if new_settings.get("theme") == "system":
                if not self.system_theme_check_timer.isActive():
                    self.system_theme_check_timer.start()
            else:
                if self.system_theme_check_timer.isActive():
                    self.system_theme_check_timer.stop()
                
            theme_manager.set_theme(new_settings["theme"])
            self.apply_theme()
            self.update_tooltips()  # Обновляем подсказки под новую тему
            
            # Обновляем тему во всех открытых диалогах
            for child in self.children():
                if isinstance(child, (SettingsDialog, NoticeDialog)):
                    child.apply_theme()
                    break
        
        # Применяем настройки таймера
        if new_settings.get("show_timer") != old_settings.get("show_timer"):
            self.show_timer_button.setChecked(new_settings["show_timer"])
            self.toggle_show_timer()
            
        # Применяем настройки "поверх всех окон"
        if new_settings.get("always_on_top") != old_settings.get("always_on_top"):
            self.always_on_top_button.setChecked(new_settings["always_on_top"])
            self.toggle_always_on_top()
            
        # Применяем настройки сетки
        if (new_settings.get("grid_h_lines") != old_settings.get("grid_h_lines") or
            new_settings.get("grid_v_lines") != old_settings.get("grid_v_lines")):
            self.grid_h_lines = new_settings["grid_h_lines"]
            self.grid_v_lines = new_settings["grid_v_lines"]
            if self.show_grid:
                self.update_image_display()
        
        # Если изменилась папка, обновляем статистику и загружаем изображения
        if new_settings.get("folder") != old_settings.get("folder"):
            current_folder = os.path.abspath(new_settings.get("folder", ""))
            if current_folder:
                # Обновляем статистику для новой папки
                if current_folder not in self.folder_stats:
                    self.folder_stats[current_folder] = 0
                self.config_manager.save_folder_stats(self.folder_stats)
                
                # Обновляем информацию в окне настроек, если оно открыто
                for child in self.children():
                    if isinstance(child, SettingsDialog):
                        child.update_folder_label(current_folder)
                        break
                        
                # Загружаем изображения из новой папки
                self.load_images(current_folder)

    def update_tooltips(self):
        """Обновляет подсказки для всех кнопок"""
        self.prev_button.setToolTip(tr("Previous image") + " (←)")
        self.next_button.setToolTip(tr("Next image") + " (→)")
        self.skip_button.setToolTip(tr("Skip") + " (S)")
        self.pause_button.setToolTip(tr("Pause") + " (Space)")
        self.open_file_button.setToolTip(tr("Open file") + " (O)")
        self.copy_image_button.setToolTip(tr("Copy image to clipboard") + " (C)")
        self.grid_button.setToolTip(tr("Toggle grid") + " (G)")
        self.delete_button.setToolTip(tr("Move to trash") + " (Delete)")
        self.bw_button.setToolTip(tr("B/W filter") + " (B)")
        self.flip_v_button.setToolTip(tr("Flip vertically") + " (V)")
        self.flip_h_button.setToolTip(tr("Flip horizontally") + " (H)")
        self.rotate_button.setToolTip(tr("Rotate 90°") + " (R)")
        self.restore_button.setToolTip(tr("Reset image") + " (Backspace)")
        self.show_timer_button.setToolTip(tr("Show/Hide timer") + " (T)")
        self.always_on_top_button.setToolTip(tr("Always on top") + " (A)")
        self.settings_button.setToolTip(tr("Settings") + " (Ctrl+,)")

    def retranslate_ui(self):
        """Обновляет все тексты в интерфейсе при смене языка"""
        # Обновляем заголовок окна
        self.setWindowTitle("GestArt")
        
        # Обновляем тексты кнопок и подсказки
        self.update_tooltips()
        
        # Обновляем текст таймера
        self.update_timer_label()

    def update_timer_label(self):
        """Обновление текста в таймере и его видимости"""
        pos = self.settings.get("timer_position", "center")
        timer_container = self.timer_label.parent()
        
        # Устанавливаем базовый стиль для таймера - без жестко заданного цвета текста
        # Кэшируем стиль, чтобы не пересоздавать его каждый раз
        if not hasattr(self, '_timer_base_style'):
            self._timer_base_style = """
                font-weight: bold;
                padding: 2px;
            """
        self.timer_label.setStyleSheet(self._timer_base_style)
        
        # Устанавливаем выравнивание для контейнера таймера
        # Кэшируем выравнивания, чтобы не пересоздавать их каждый раз
        if not hasattr(self, '_timer_alignments'):
            self._timer_alignments = {
                "left": Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignTop,
                "right": Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTop,
                "center": Qt.AlignmentFlag.AlignHCenter | Qt.AlignmentFlag.AlignTop
            }
        
        alignment = self._timer_alignments.get(pos, self._timer_alignments["center"])
        self.overlay_widget.layout().setAlignment(timer_container, alignment)
            
        if self.settings.get("show_timer", True):
            if self.settings.get("unlimited_time", False):
                session_text = tr("Session")
                accepted_count = str(self.accepted_count)
                
                if not self.settings.get("unlimited_images", False):
                    num_images = str(self.settings['num_images'])
                    timer_text = f"{session_text}: {accepted_count}/{num_images}"
                else:
                    timer_text = f"{session_text}: {accepted_count}"
                    
                self.timer_label.setText(timer_text)
            else:
                session_text = tr("Session")
                accepted_count = str(self.accepted_count)
                remaining_text = tr("Remaining")
                remaining_time = str(self.remaining_time)
                sec_text = tr("sec")
                
                if not self.settings.get("unlimited_images", False):
                    num_images = str(self.settings['num_images'])
                    session_part = f"{session_text}: {accepted_count}/{num_images}"
                else:
                    session_part = f"{session_text}: {accepted_count}"
                    
                remaining_part = f"{remaining_text}: {remaining_time} {sec_text}"
                
                # Если текст получается длинным, добавляем перенос строки
                if len(session_part) + len(remaining_part) > 40:
                    timer_text = f"{session_part}\n{remaining_part}"
                else:
                    timer_text = f"{session_part} | {remaining_part}"
                
                self.timer_label.setText(timer_text)
            
            # Настройка обработчика нажатия в зависимости от состояния
            if self.is_in_break:
                # В перерыве - позволяем пропустить перерыв по клику
                self.timer_label.setCursor(Qt.CursorShape.PointingHandCursor)
                self.timer_label.mousePressEvent = lambda event: self.skip_break()
            elif self.is_session_completed:
                # Сессия завершена - позволяем начать новую по клику
                self.timer_label.setCursor(Qt.CursorShape.PointingHandCursor)
                self.timer_label.mousePressEvent = lambda event: self.start_new_session()
            else:
                # Активная сессия - отключаем обработку кликов
                self.timer_label.setCursor(Qt.CursorShape.ArrowCursor)
                self.timer_label.mousePressEvent = lambda event: None
                
            self.timer_label.show()
        else:
            self.timer_label.hide()

    def load_images(self, folder):
        """Загружает изображения из выбранной папки"""
        if os.path.isdir(folder):
            self.scanner_thread = FolderScannerThread(folder, self.supported_extensions)
            self.scanner_thread.scanned.connect(self.on_images_scanned)
            self.scanner_thread.error.connect(self.on_scanner_error)
            self.scanner_thread.progress.connect(self.on_progress)
            self.scanner_thread.start()
            
            # Обновляем статистику для текущей папки
            if folder not in self.folder_stats:
                self.folder_stats[folder] = 0
            # Сохраняем обновленную статистику
            self.config_manager.save_folder_stats(self.folder_stats)

    def on_images_scanned(self, files):
        """Обработчик завершения сканирования папки"""
        self.image_files = files
        
        # Скрываем индикатор прогресса
        self.progress_label.hide()
        
        if not files:
            NoticeDialog.show_warning(self, tr("Warning"), tr("No images found in selected folder and subfolders"))
            return
            
        # Логируем количество найденных изображений
        logging.info(f"Found images: {len(files)}")
            
        # Обновляем статистику для текущей папки
        current_folder = os.path.abspath(self.settings.get("folder", ""))
        if current_folder not in self.folder_stats:
            self.folder_stats[current_folder] = 0
        self.config_manager.save_folder_stats(self.folder_stats)
        
        # Если открыто окно настроек, обновляем в нем информацию
        for child in self.children():
            if isinstance(child, SettingsDialog):
                child.update_folder_label(current_folder)
                break
        
        # Если это первая загрузка папки, начинаем сессию
        if not self.current_image_path and not self.settings.get("folder"):
            self.start_session()

    def on_scanner_error(self, error_message):
        if "no images" in error_message.lower():
            NoticeDialog.show_warning(self, tr("Warning"), error_message)
        else:
            NoticeDialog.show_error(self, tr("Error"), error_message)
        logging.error(error_message)

    def on_progress(self, processed_files, total_files):
        progress = min(int((processed_files / total_files) * 100), 100)
        self.progress_label.setText(f"{tr('Scanning')}: {progress}%")
        self.progress_label.show()

    def start_session(self):
        """Начинает новую сессию с загрузкой изображений"""
        # Проверяем, не в режиме перерыва ли мы
        if hasattr(self, 'is_in_break') and self.is_in_break:
            self.is_in_break = False
            self.break_timer.stop()
            
        self.is_session_completed = False  # Сбрасываем флаг завершения сессии
        self.is_paused = False
        
        # Обновляем состояние кнопки паузы с иконкой
        self.pause_button.setIcon(create_themed_icon("resources/pause0.png"))
        self.pause_button.setToolTip(tr("Pause") + " (Space)")
        
        if len(self.image_files) == 0:
            # Показываем логотип меню, если нет изображений
            if hasattr(self, 'menu_logo'):
                self.menu_logo.show()
                
            self.timer_label.setText(tr("No images to display. Select a folder.") + 
                                   "\n" + tr("Try using B, V, H, R, and G keys to experiment with effects on the logo."))
            self.timer_label.setCursor(Qt.CursorShape.ArrowCursor)
            self.timer_label.mousePressEvent = None
            return
            
        # Сохраняем предыдущую сессию перед созданием новой, если она существует
        previous_session_images = None
        if self.session_images and len(self.session_images) > 0:
            previous_session_images = self.session_images.copy()
            
        # Сбрасываем счетчик просмотренных изображений
        self.accepted_count = 0  # Начинаем с 0, первое изображение увеличит до 1
        self.session_images = []  # Новый список для текущей сессии
        
        # Устанавливаем или сбрасываем время
        self.remaining_time = self.settings["display_time"]
        
        # Обновляем метку времени
        self.update_timer_label()
        
        # Проверяем, включен ли таймер
        if self.countdown_timer.isActive():
            self.countdown_timer.stop()
        
        # Показываем следующее изображение, чтобы начать сессию
        self.display_next_image(increment=True)

    def start_new_session(self, event=None):
        """Запускает новую сессию"""
        # Если сессия уже идет, останавливаем её
        if hasattr(self, 'countdown_timer') and self.countdown_timer.isActive():
            self.countdown_timer.stop()
            self.is_paused = True
            self.pause_button.setIcon(create_themed_icon("resources/play0.png"))
            self.pause_button.setToolTip(tr("Resume") + " (Space)")
            
        # Сбрасываем состояние
        self.current_image_index = 0
        self.is_paused = False
        self.pause_button.setIcon(create_themed_icon("resources/pause0.png"))
        self.pause_button.setToolTip(tr("Pause") + " (Space)")
        
        # Запускаем новую сессию
        self.start_session()

    def apply_image_effects(self, pixmap):
        current = pixmap.copy()
        
        # Применяем ч/б фильтр
        if self.is_bw:
            image = current.toImage().convertToFormat(QImage.Format.Format_Grayscale8)
            current = QPixmap.fromImage(image)
            
        # Применяем отражения
        transform = QTransform()
        if self.flip_v_active:
            transform.scale(1, -1)
        if self.flip_h_active:
            transform.scale(-1, 1)
            
        # Применяем поворот
        if self.rotation_angle != 0:
            transform.rotate(self.rotation_angle)
            
        if self.flip_v_active or self.flip_h_active or self.rotation_angle != 0:
            current = current.transformed(transform)
            
        return current

    def pause(self):
        """Переключение режима паузы"""
        self.is_paused = not self.is_paused
        if self.is_paused:
            self.countdown_timer.stop()
            self.pause_button.setIcon(create_themed_icon("resources/play0.png"))
            self.pause_button.setToolTip(tr("Resume") + " (Space)")
            
            # Останавливаем звук таймера, если он играет
            if self.timer_sound_playing:
                self.timer_sound_player.stop()
                self.timer_sound_playing = False
        else:
            if not self.settings.get("unlimited_time", False):
                self.countdown_timer.start()
                self.pause_button.setIcon(create_themed_icon("resources/pause0.png"))
                self.pause_button.setToolTip(tr("Pause") + " (Space)")

    def update_countdown(self):
        """Обновление таймера"""
        if self.remaining_time > 0:
            self.remaining_time -= 1
            self.update_timer_label()
            
            # Воспроизводим звук за 3.5 секунды до окончания таймера
            if self.remaining_time <= 3 and not self.timer_sound_playing and not self.settings.get("unlimited_time", False):
                # Устанавливаем громкость в соответствии с настройками
                volume = self.settings.get("timer_volume", 50) / 100.0
                self.timer_audio_output.setVolume(volume)
                self.timer_sound_player.play()
                self.timer_sound_playing = True
            
            # Если время вышло, проверяем можно ли показать следующее изображение
            if self.remaining_time == 0:
                self.timer_sound_playing = False  # Сбрасываем флаг для следующего изображения
                # Проверяем, не будет ли следующее изображение превышать лимит
                if not self.settings.get("unlimited_images", False) and self.accepted_count >= self.settings["num_images"]:
                    self.is_session_completed = True
                    self.countdown_timer.stop()
                    self.end_session()
                else:
                    self.display_next_image()

    def display_next_image(self, increment=True):
        # Проверяем лимит изображений перед загрузкой следующего
        if increment and not self.settings.get("unlimited_images", False):
            if self.accepted_count >= self.settings["num_images"]:
                self.is_session_completed = True
                self.end_session()
                return

        # Сбрасываем флаг воспроизведения звука при переходе к новому изображению
        self.timer_sound_playing = False

        # Получаем список доступных изображений
        if self.settings["save_history"]:
            # Если история включена, исключаем просмотренные изображения
            available = [img for img in self.image_files if img not in self.displayed_history]
            # Если все изображения просмотрены, очищаем историю и показываем уведомление
            if not available:
                NoticeDialog.show_warning(self, tr("Warning"), tr("All images have been viewed. Starting over."))
                self.displayed_history.clear()
                available = self.image_files.copy()
        else:
            # Если история выключена, используем все изображения
            available = self.image_files.copy()

        if available:
            next_image = random.choice(available)
            self.current_image_path = next_image
            self.display_image(next_image)
            
            if increment:
                self.accepted_count += 1
                # Добавляем изображение в сессию только если его там еще нет
                if next_image not in self.session_images:
                    self.session_images.append(next_image)
                self.history_index = len(self.session_images) - 1
                
                # Добавляем в глобальную историю только если это новое изображение
                if self.settings["save_history"] and next_image not in self.displayed_history:
                    self.displayed_history.append(next_image)
                    # Сохраняем историю после добавления нового изображения
                    self.config_manager.save_history(self.displayed_history)
                    
                    # Обновляем статистику просмотров для текущей папки
                    current_folder = os.path.dirname(next_image)
                    if current_folder not in self.folder_stats:
                        self.folder_stats[current_folder] = 0
                    self.folder_stats[current_folder] += 1
                    self.config_manager.save_folder_stats(self.folder_stats)
            
            self.remaining_time = self.settings["display_time"]
            self.update_timer_label()
            
            # Проверяем, должен ли таймер быть видимым
            if self.settings.get("show_timer", True):
                self.timer_label.show()
            else:
                self.timer_label.hide()
                
            # Запускаем таймер, если не в режиме без ограничений
            if not self.settings.get("unlimited_time", False) and not self.is_paused:
                self.countdown_timer.start()

    def show_previous_image(self):
        # Сбрасываем флаг воспроизведения звука при переходе к предыдущему изображению
        self.timer_sound_playing = False
        
        # Отладочная информация
        logging.info(f"show_previous_image: history_index={self.history_index}, session_images_count={len(self.session_images)}, displayed_history_count={len(self.displayed_history)}")
        if self.session_images:
            logging.info(f"Current session_images: {[os.path.basename(img) for img in self.session_images]}")
        if self.displayed_history:
            logging.info(f"Recent displayed_history: {[os.path.basename(img) for img in self.displayed_history[-5:]]}")
        
        # Если мы в текущей сессии и есть предыдущие изображения
        if self.history_index > 0:
            self.history_index -= 1
            path = self.session_images[self.history_index]
            
            # Проверяем существование файла
            if not os.path.exists(path) or not os.access(path, os.R_OK):
                # Файл был удален или недоступен - удаляем из сессии
                logging.warning(f"Unavailable image in session when going back: {path}")
                self.remove_invalid_image(path)
                # Рекурсивно пытаемся показать предыдущее изображение
                return self.show_previous_image()
            
            # Показываем изображение
            self.current_image_path = path
            self.display_image(path)
            self.remaining_time = self.settings["display_time"]
            # Обновляем счетчик просмотренных изображений для корректного отображения
            self.accepted_count = self.history_index + 1
            self.update_timer_label()
            
        # Если мы в начале текущей сессии, но есть история просмотра из прошлых сессий
        elif len(self.displayed_history) > 0 and self.settings["save_history"] and len(self.image_files) > 0:
            # Получаем список действительных изображений из истории просмотра
            # Используем только те изображения, которые есть и в истории, и в текущей папке
            history_images = [img for img in self.displayed_history if img in self.image_files]
            
            if history_images:
                # Создаем список валидных изображений из истории (в правильном хронологическом порядке)
                valid_history_images = []
                for img in history_images:  # Убираем reversed() для правильного порядка
                    if os.path.exists(img) and os.access(img, os.R_OK):
                        if img not in self.session_images:
                            valid_history_images.append(img)
                
                if valid_history_images:
                    # Сохраняем текущие изображения сессии
                    current_session = self.session_images.copy()
                    
                    # Добавляем изображения из истории в начало сессии
                    self.session_images = valid_history_images + current_session
                    
                    # Устанавливаем индекс на последнее изображение из истории (самое недавнее)
                    self.history_index = len(valid_history_images) - 1
                    
                    # Отладочная информация
                    logging.info(f"Added history images to session. New session order: {[os.path.basename(img) for img in self.session_images]}")
                    logging.info(f"Set history_index to: {self.history_index}")
                    
                    # Показываем изображение
                    path = self.session_images[self.history_index]
                    self.current_image_path = path
                    self.display_image(path)
                    self.remaining_time = self.settings["display_time"]
                    self.accepted_count = self.history_index + 1
                    self.update_timer_label()
                    return
                else:
                    # Все изображения из истории уже в текущей сессии
                    # Ищем самое раннее изображение в текущей сессии, которое есть в истории
                    # Ищем с начала сессии (самые старые изображения)
                    for i, img in enumerate(self.session_images):
                        if img in history_images and os.path.exists(img) and os.access(img, os.R_OK):
                            self.history_index = i
                            self.current_image_path = img
                            self.display_image(img)
                            self.remaining_time = self.settings["display_time"]
                            self.accepted_count = self.history_index + 1
                            self.update_timer_label()
                            return
            
            # Если не удалось найти предыдущее изображение в истории
            NoticeDialog.show_warning(self, tr("Warning"), tr("No previous image"))
        else:
            NoticeDialog.show_warning(self, tr("Warning"), tr("No previous image"))

    def show_next_image(self):
        # Сбрасываем флаг воспроизведения звука при переходе к следующему изображению
        self.timer_sound_playing = False
        
        # Отладочная информация
        logging.info(f"show_next_image: history_index={self.history_index}, session_images_count={len(self.session_images)}, displayed_history_count={len(self.displayed_history)}")
        if self.session_images:
            logging.info(f"Current session_images: {[os.path.basename(img) for img in self.session_images]}")
        if self.displayed_history:
            logging.info(f"Recent displayed_history: {[os.path.basename(img) for img in self.displayed_history[-5:]]}")
            
        if self.history_index < len(self.session_images) - 1:
            self.history_index += 1
            path = self.session_images[self.history_index]
            
            # Проверяем существование файла
            if not os.path.exists(path) or not os.access(path, os.R_OK):
                # Файл был удален или недоступен - удаляем из истории и сессии
                logging.warning(f"Unavailable image in history when going forward: {path}")
                self.remove_invalid_image(path)
                # Не уменьшаем history_index, так как remove_invalid_image уже сделал это при необходимости
                
                # Рекурсивно пытаемся показать следующее изображение
                return self.show_next_image()
            
            self.current_image_path = path
            self.display_image(path)
            self.remaining_time = self.settings["display_time"]
            # Обновляем счетчик просмотренных изображений для корректного отображения
            self.accepted_count = self.history_index + 1
            self.update_timer_label()
        else:
            self.display_next_image(increment=True)

    def skip_image(self):
        """Пропускает текущее изображение без увеличения счетчика и добавления в session_images"""
        self.timer_sound_playing = False
        
        if not self.image_files:
            return # Нечего пропускать

        available_images = []
        if self.settings.get("save_history", True):
            # История включена: нужны изображения не из истории
            # (кроме текущего, если оно уже в истории - его можно пропустить на новое)
            seen_images = self.displayed_history.copy()
            available_images = [img for img in self.image_files if img not in seen_images]
            
            if not available_images and self.image_files: 
                # Если все просмотрены, но файлы есть - начинаем заново
                NoticeDialog.show_warning(self, tr("Warning"), tr("All images have been viewed. Starting over."))
            self.displayed_history.clear()
            # Теперь доступны все, кроме текущего (если он единственный)
            available_images = [img for img in self.image_files if img != self.current_image_path]
            if not available_images and self.image_files: # Если текущий был единственным
                available_images = self.image_files[:1] # Берем первый
        else:
            # История выключена: нужны все изображения, кроме текущего
            available_images = [img for img in self.image_files if img != self.current_image_path]

        if available_images:
            next_image = random.choice(available_images)
            self.current_image_path = next_image # Обновляем путь
            
            # Добавляем пропущенное в историю, если нужно
            if self.settings.get("save_history", True) and self.current_image_path not in self.displayed_history:
                 self.displayed_history.append(self.current_image_path)
                 # Сохраняем историю
                 self.config_manager.save_history(self.displayed_history)
            
            # Отображаем новое изображение
            self.display_image(next_image)
            self.remaining_time = self.settings["display_time"]
            self.update_timer_label()
            
            # Запускаем таймер, если не в режиме без ограничений и не на паузе
            if not self.settings.get("unlimited_time", False) and not self.is_paused:
                self.countdown_timer.start()
        elif self.image_files: # Если доступны только текущее изображение
             NoticeDialog.show_warning(self, tr("Warning"), tr("No other images available to skip to."))
        else: # Если вообще нет изображений
             NoticeDialog.show_warning(self, tr("Warning"), tr("No images available to skip to"))

    def display_image(self, image_path):
        """Отображает изображение в интерфейсе"""
        try:
            if not image_path:
                return
            if hasattr(self, 'menu_logo'):
                self.menu_logo.hide()
                
            if not os.path.exists(image_path):
                logging.warning(f"File does not exist: {image_path}")
                self.handle_load_error(image_path, "File does not exist")
                return
                
            if not os.access(image_path, os.R_OK):
                logging.warning(f"No access to file: {image_path}")
                self.handle_load_error(image_path, "No access to file")
                return
                
            self.current_image_path = image_path
                
            if image_path in self.pixmap_cache:
                # Обновляем порядок доступа для LRU
                if image_path in self.cache_access_order:
                    self.cache_access_order.remove(image_path)
                self.cache_access_order.append(image_path)
                
                self.original_pixmap = self.pixmap_cache[image_path]
                self.current_pixmap = self.apply_image_effects(self.original_pixmap)
                self.zoom_factor = 1.0
                self.update_image_display()
                self.load_error_streak = 0 # Reset error streak on success
            else:
                self.cleanup_loader(image_path)
                loader = ImageLoaderThread(image_path, self)
                loader.loaded.connect(self.on_image_loaded)
                loader.error.connect(self.on_image_load_error)
                self.image_loaders[image_path] = loader
                loader.start()

        except Exception as e:
            logging.error(f"Error in display_image for {image_path}: {e}")
            self.handle_load_error(image_path, str(e))

    def cleanup_loader(self, image_path):
        """Безопасно очищает загрузчик изображения"""
        if image_path in self.image_loaders:
            try:
                loader = self.image_loaders[image_path]
                if loader.isRunning():
                    loader.quit()
                    if not loader.wait(1000):  # Ждем максимум 1 секунду
                        loader.terminate()
                        loader.wait()
                del self.image_loaders[image_path]
            except Exception as e:
                logging.error(f"Error cleaning up loader for {image_path}: {e}")

    def remove_invalid_image(self, image_path):
        """Удаляет недействительное изображение из всех коллекций"""
        try:
            # Запоминаем индекс изображения в session_images
            session_index = -1
            if image_path in self.session_images:
                session_index = self.session_images.index(image_path)
            
            # Удаляем из всех коллекций
            if image_path in self.image_files:
                self.image_files.remove(image_path)
            if image_path in self.displayed_history:
                self.displayed_history.remove(image_path)
            if image_path in self.pixmap_cache:
                del self.pixmap_cache[image_path]
            
            # Особая обработка для session_images
            if session_index >= 0:
                self.session_images.pop(session_index)
                
                # Корректируем history_index
                if session_index <= self.history_index:
                    self.history_index = max(0, self.history_index - 1)
            
            self.cleanup_loader(image_path)
            
            # Сохраняем обновленную историю
            if self.settings["save_history"]:
                self.config_manager.save_history(self.displayed_history)
                
        except Exception as e:
            logging.error(f"Error removing image {image_path}: {e}")

    def on_image_loaded(self, image_path, pixmap):
        try:
            # Reset error streak on successful load
            self.load_error_streak = 0
            
            if not pixmap or pixmap.isNull():
                self.on_image_load_error(image_path, "Loaded image is invalid")
                return

            self.cleanup_loader(image_path)

            if self.current_image_path != image_path:
                return

            self.original_pixmap = pixmap
            
            # LRU кэширование - удаляем самый старый элемент
            if len(self.pixmap_cache) >= self.max_cache_size:
                if self.cache_access_order:
                    oldest_key = self.cache_access_order.pop(0)
                    if oldest_key in self.pixmap_cache:
                        del self.pixmap_cache[oldest_key]
            
            self.pixmap_cache[image_path] = pixmap
            self.cache_access_order.append(image_path)

            self.current_pixmap = self.apply_image_effects(self.original_pixmap)
            self.zoom_factor = 1.0
            self.update_image_display()

        except Exception as e:
            logging.error(f"Error in on_image_loaded for {image_path}: {e}")
            self.on_image_load_error(image_path, str(e))

    def on_image_load_error(self, image_path, error_message):
        try:
            logging.error(f"Error loading image {image_path}: {error_message}")
            self.cleanup_loader(image_path)
            self.handle_load_error(image_path, error_message)
        except Exception as e:
            logging.error(f"Error in on_image_load_error handler for {image_path}: {e}")

    def handle_load_error(self, image_path, error_message):
        """Обрабатывает ошибки загрузки изображений"""
        self.load_error_streak += 1
        logging.error(f"Error loading image {image_path}: {error_message}")
        
        if self.load_error_streak >= self.max_load_error_streak:
            NoticeDialog.show_error(self, tr("Error"), tr("No more valid images found in the folder."))
            self.is_session_completed = True
            self.end_session()  # Go to end session state (break or completion message)
        else:
            # Schedule next image display without recursion
            QTimer.singleShot(0, lambda: self.display_next_image(increment=False))

    def resizeEvent(self, event):
        super().resizeEvent(event)
        # Обновляем размеры оверлея и контейнера
        self.overlay_widget.setGeometry(0, 0, self.width(), self.height())
        self.image_container.setGeometry(0, 0, self.width(), self.height())
        # Обновляем отображение изображения
        self.update_image_display()
        
        # Обновляем позицию индикатора зума, если он видим
        if self.zoom_indicator.isVisible():
            self.zoom_indicator.move(
                self.width() - self.zoom_indicator.width() - 20,
                self.height() - self.zoom_indicator.height() - 20
            )

    def open_current_file(self):
        if not self.current_image_path:
            return
        
        normalized_path = os.path.normpath(self.current_image_path)
        if not os.path.exists(normalized_path):
            NoticeDialog.show_error(self, tr("Error"), tr("File is already deleted or inaccessible"))
            return
            
        try:
            os.startfile(normalized_path)
        except Exception as e:
            NoticeDialog.show_error(self, tr("Error"), f"{tr('Cannot open file')}: {e}")

    def delete_current_file(self):
        if not self.current_image_path:
            return
            
        normalized_path = os.path.normpath(self.current_image_path)
            
        if not os.path.exists(normalized_path):
            NoticeDialog.show_error(self, tr("Error"), tr("File is already deleted or inaccessible"))
            return
            
        try:
            # Проверяем настройку подтверждения удаления
            should_delete = True
            if self.settings.get("confirm_delete", True):
                dialog = DeleteConfirmationDialog(self)
                should_delete = (dialog.exec() == QDialog.DialogCode.Accepted)
            
            if should_delete:
                # Сохраняем текущий индекс в истории перед удалением
                current_index = self.history_index
                
                send2trash(normalized_path)
                
                # Удаляем из всех коллекций
                if self.current_image_path in self.image_files:
                    self.image_files.remove(self.current_image_path)
                if self.current_image_path in self.displayed_history:
                    self.displayed_history.remove(self.current_image_path)
                if self.current_image_path in self.pixmap_cache:
                    del self.pixmap_cache[self.current_image_path]
                
                # Отображаем следующее изображение, если доступно
                if self.image_files:
                    self.display_next_image(increment=False)
                else:
                    self.is_session_completed = True
                    self.end_session()
        except Exception as e:
            NoticeDialog.show_error(self, tr("Error"), tr("Failed to move file to trash") + f": {e}")

    def apply_bw_filter(self):
        """Применяет черно-белый фильтр к изображению"""
        self.is_bw = self.bw_button.isChecked()
        if self.is_bw:
            self.bw_button.setIcon(create_themed_icon("resources/bwfilter1.png"))  # Используем обновленную функцию
        else:
            self.bw_button.setIcon(create_themed_icon("resources/bwfilter0.png"))  # Используем обновленную функцию
        
        if self.current_pixmap:
            self.current_pixmap = self.apply_image_effects(self.original_pixmap)
            self.update_image_display()
        elif hasattr(self, 'menu_logo') and self.menu_logo.isVisible():
            # Применяем эффект к логотипу при отсутствии изображений
            self.menu_logo.setPixmap(self.apply_image_effects(self.menu_logo_pixmap))

    def flip_vertical(self):
        self.flip_v_active = self.flip_v_button.isChecked()
        if self.current_pixmap:
            self.current_pixmap = self.apply_image_effects(self.original_pixmap)
            self.update_image_display()
        elif hasattr(self, 'menu_logo') and self.menu_logo.isVisible():
            # Применяем эффект к логотипу при отсутствии изображений
            self.menu_logo.setPixmap(self.apply_image_effects(self.menu_logo_pixmap))

    def flip_horizontal(self):
        self.flip_h_active = self.flip_h_button.isChecked()
        if self.current_pixmap:
            self.current_pixmap = self.apply_image_effects(self.original_pixmap)
            self.update_image_display()
        elif hasattr(self, 'menu_logo') and self.menu_logo.isVisible():
            # Применяем эффект к логотипу при отсутствии изображений
            self.menu_logo.setPixmap(self.apply_image_effects(self.menu_logo_pixmap))

    def rotate_90(self):
        self.rotation_angle = (self.rotation_angle + 90) % 360
        if self.current_pixmap:
            self.current_pixmap = self.apply_image_effects(self.original_pixmap)
            self.update_image_display()
        elif hasattr(self, 'menu_logo') and self.menu_logo.isVisible():
            # Применяем эффект к логотипу при отсутствии изображений
            self.menu_logo.setPixmap(self.apply_image_effects(self.menu_logo_pixmap))

    def restore_original(self):
        self.is_bw = False
        self.flip_v_active = False
        self.flip_h_active = False
        self.rotation_angle = 0
        self.zoom_factor = 1.0  # Сбрасываем масштаб
        self.zoom_center = None  # Сбрасываем центр зума
        self.bw_button.setChecked(False)
        self.flip_v_button.setChecked(False)
        self.flip_h_button.setChecked(False)
        
        if self.original_pixmap:
            self.current_pixmap = self.original_pixmap.copy()
            self.update_image_display()
        elif hasattr(self, 'menu_logo') and self.menu_logo.isVisible():
            # Восстанавливаем оригинальный логотип
            self.menu_logo.setPixmap(self.menu_logo_pixmap)

    def toggle_show_timer(self):
        """Переключает отображение таймера"""
        is_visible = self.show_timer_button.isChecked()
        self.settings["show_timer"] = is_visible
        self.config_manager.save_settings(self.settings)
        
        timer_container = self.timer_label.parent()
        
        if is_visible:
            self.show_timer_button.setIcon(create_themed_icon("resources/timer1.png"))  # Используем обновленную функцию
            timer_container.show()
            self.timer_label.show()
        else:
            self.show_timer_button.setIcon(create_themed_icon("resources/timer0.png"))  # Используем обновленную функцию
            timer_container.hide()
            self.timer_label.hide()

    def toggle_always_on_top(self):
        """Включает/выключает режим "Поверх всех окон" """
        always_on_top = self.always_on_top_button.isChecked()
        self.settings["always_on_top"] = always_on_top
        self.config_manager.save_settings(self.settings)
        
        if always_on_top:
            self.always_on_top_button.setIcon(create_themed_icon("resources/pin1.png"))  # Используем обновленную функцию
            self.setWindowFlags(self.windowFlags() | Qt.WindowType.WindowStaysOnTopHint)
        else:
            self.always_on_top_button.setIcon(create_themed_icon("resources/pin0.png"))  # Используем обновленную функцию
            self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowStaysOnTopHint)
        
        # Нужно показать окно снова после изменения флагов
        self.show()
        
    def toggle_grid(self):
        """Включает/выключает отображение сетки"""
        self.show_grid = self.grid_button.isChecked()
        
        if self.show_grid:
            self.grid_button.setIcon(create_themed_icon("resources/grid1.png"))  # Используем обновленную функцию
        else:
            self.grid_button.setIcon(create_themed_icon("resources/grid0.png"))  # Используем обновленную функцию
            
        self.update_image_display()
        
        # Применяем сетку к логотипу, если нет загруженных изображений
        if hasattr(self, 'menu_logo') and self.menu_logo.isVisible():
            # Создаем новый QPixmap для отображения логотипа с сеткой
            logo_width = self.menu_logo.width()
            logo_height = self.menu_logo.height()
            final_pixmap = QPixmap(logo_width, logo_height)
            final_pixmap.fill(Qt.GlobalColor.transparent)
            
            painter = QPainter(final_pixmap)
            
            # Рисуем логотип с примененными эффектами
            logo_with_effects = self.apply_image_effects(self.menu_logo_pixmap)
            painter.drawPixmap(0, 0, logo_with_effects)
            
            # Если включена сетка, рисуем её поверх логотипа
            if self.show_grid:
                self.draw_grid(painter, logo_width, logo_height)
                
            painter.end()
            self.menu_logo.setPixmap(final_pixmap)

    def end_session(self):
        """Завершение сессии"""
        self.countdown_timer.stop()
        # Освобождаем память от загруженных изображений, которые сейчас не отображаются
        
        # Если включены перерывы, начинаем перерыв
        if self.settings.get("use_break", False):
            self.is_in_break = True
            
            # Устанавливаем время перерыва из настроек
            self.break_remaining_time = self.settings["break_duration"]
            
            # Переводим отдельные части
            break_text = tr("Break")
            sec_text = tr("sec")
            skip_text = tr("Press Space to skip")
            
            # Собираем полный текст
            timer_text = f"{break_text}: {self.break_remaining_time} {sec_text}... {skip_text}"
            
            self.timer_label.setText(timer_text)
            self.timer_label.setCursor(Qt.CursorShape.PointingHandCursor)
            self.timer_label.mousePressEvent = lambda event: self.skip_break()
            
            # Останавливаем таймер, если он активен
            if self.break_timer.isActive():
                self.break_timer.stop()
                
            # Настраиваем таймер заново
            self.break_timer = QTimer()
            self.break_timer.setInterval(1000)  # 1 секунда
            self.break_timer.timeout.connect(self.update_break_countdown)
            self.break_timer.start()
        else:
            self.is_in_break = False
            self.is_session_completed = True  # Сессия завершена без перерыва
            # Напрямую устанавливаем текст, а не через update_timer_label,
            # так как функция update_timer_label обрабатывает разные состояния по-разному
            
            # Переводим отдельные части
            completed_text = tr("Session completed")
            start_new_text = tr("Press Space to start new session")
            
            # Собираем полный текст
            timer_text = f"{completed_text}. {start_new_text}"
            self.timer_label.setText(timer_text)
            self.timer_label.setCursor(Qt.CursorShape.PointingHandCursor)
            self.timer_label.mousePressEvent = lambda event: self.start_new_session()

    def update_break_countdown(self):
        """Обновление таймера перерыва"""
        if not self.is_in_break:
            self.break_timer.stop()
            return
            
        self.break_remaining_time -= 1
        
        # Переводим отдельные части
        break_text = tr("Break")
        sec_text = tr("sec")
        skip_text = tr("Press Space to skip")
        
        # Собираем полный текст
        timer_text = f"{break_text}: {self.break_remaining_time} {sec_text}... {skip_text}"
        self.timer_label.setText(timer_text)
        
        if self.break_remaining_time <= 0:
            self.break_timer.stop()
            self.is_in_break = False
            QTimer.singleShot(100, self.start_session)  # Запускаем с небольшой задержкой

    def skip_break(self, event=None):
        """Пропуск перерыва"""
        if not self.is_in_break:
            return
            
        if self.break_timer.isActive():
            self.break_timer.stop()
                
        self.is_in_break = False
        # Сразу запускаем новую сессию
        QTimer.singleShot(100, self.start_session)

    def handle_space_key(self):
        """Обработка нажатия пробела"""
        if self.is_in_break:
            # В режиме перерыва - пропускаем перерыв
            self.skip_break()
        elif self.is_session_completed:
            # Если сессия завершена - начинаем новую
            self.start_session()
        elif self.is_paused:
            # Если на паузе - возобновляем
            self.toggle_pause()
        else:
            # Если активная сессия - ставим на паузу
            self.toggle_pause()

    def toggle_pause(self):
        """Переключение режима паузы"""
        self.is_paused = not self.is_paused
        if self.is_paused:
            self.countdown_timer.stop()
            self.pause_button.setIcon(create_themed_icon("resources/play0.png"))
            self.pause_button.setToolTip(tr("Resume") + " (Space)")
            
            # Останавливаем звук таймера, если он играет
            if self.timer_sound_playing:
                self.timer_sound_player.stop()
                self.timer_sound_playing = False
        else:
            if not self.settings.get("unlimited_time", False):
                self.countdown_timer.start()
                self.pause_button.setIcon(create_themed_icon("resources/pause0.png"))
                self.pause_button.setToolTip(tr("Pause") + " (Space)")

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton and self.zoom_factor > 1.0:
            self.is_dragging = True
            self.last_mouse_pos = event.pos()
            # Меняем курсор при перетаскивании
            self.setCursor(Qt.CursorShape.ClosedHandCursor)
        super().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self.is_dragging = False
            # Восстанавливаем стандартный курсор
            self.setCursor(Qt.CursorShape.ArrowCursor)
        super().mouseReleaseEvent(event)

    def mouseMoveEvent(self, event):
        if self.is_dragging and self.last_mouse_pos:
            delta = event.pos() - self.last_mouse_pos
            self.pan_image(delta)
            self.last_mouse_pos = event.pos()
        super().mouseMoveEvent(event)

    def pan_image(self, delta):
        if self.zoom_factor <= 1.0:
            return
            
        # Обновляем переменные перемещения
        self.pan_x += delta.x()
        self.pan_y += delta.y()
        
        self.update_image_display()

    def wheelEvent(self, event):
        # Получаем позицию мыши относительно окна
        mouse_pos = event.position().toPoint()
        
        # Изменяем масштаб
        delta = event.angleDelta().y()
        prev_zoom = self.zoom_factor
        
        # Рассчитываем точку, на которую наведен курсор в координатах изображения
        view_width = self.image_label.width()
        view_height = self.image_label.height()
        
        if self.current_pixmap:
            scaled_pixmap = self.current_pixmap.scaled(
                view_width, view_height,
                Qt.AspectRatioMode.KeepAspectRatio, 
                Qt.TransformationMode.SmoothTransformation
            )
            
            img_x = (view_width - scaled_pixmap.width()) // 2
            img_y = (view_height - scaled_pixmap.height()) // 2
            
            # Координаты мыши относительно изображения
            rel_x = (mouse_pos.x() - img_x)
            rel_y = (mouse_pos.y() - img_y)
            
            # Нормализованные координаты (от 0 до 1) в пределах изображения
            if scaled_pixmap.width() > 0 and scaled_pixmap.height() > 0:
                norm_x = max(0, min(1, rel_x / scaled_pixmap.width()))
                norm_y = max(0, min(1, rel_y / scaled_pixmap.height()))
            else:
                norm_x = 0.5
                norm_y = 0.5
        else:
            norm_x = 0.5
            norm_y = 0.5
            
        # Рассчитываем новый масштаб
        if delta > 0:
            self.zoom_factor *= 1.1
        else:
            self.zoom_factor *= 0.9
            
        # Ограничиваем масштаб (повышаем минимум до 0.5)
        self.zoom_factor = max(0.5, min(10.0, self.zoom_factor))
        
        # Если изображение уменьшилось до нормального размера, устанавливаем ровно 1.0
        if prev_zoom > 1.0 and self.zoom_factor <= 1.0:
            self.zoom_factor = 1.0
            self.pan_x = 0
            self.pan_y = 0
        
        # Регулируем перемещение так, чтобы точка под курсором оставалась на месте
        if prev_zoom > 1.0 and self.zoom_factor > 1.0 and self.current_pixmap:
            # Только выполняем коррекцию, если зум был активен до и после изменения
            # Находим как изменилось соотношение масштабов
            zoom_ratio = self.zoom_factor / prev_zoom
            
            # Корректируем pan на основе нормализованных координат и изменения масштаба
            scaled_pixmap_width = scaled_pixmap.width()
            scaled_pixmap_height = scaled_pixmap.height()
            
            # Корректируем перемещение, чтобы точка под курсором оставалась на месте
            old_x_offset = norm_x * scaled_pixmap_width * prev_zoom
            old_y_offset = norm_y * scaled_pixmap_height * prev_zoom
            
            new_x_offset = norm_x * scaled_pixmap_width * self.zoom_factor
            new_y_offset = norm_y * scaled_pixmap_height * self.zoom_factor
            
            self.pan_x -= (new_x_offset - old_x_offset)
            self.pan_y -= (new_y_offset - old_y_offset)
        
        self.update_image_display()
        self.show_zoom_indicator()  # Показываем индикатор текущего масштаба
        event.accept()

    def update_image_display(self):
        if not hasattr(self, 'current_pixmap') or not self.current_pixmap:
            return
        
        # Если масштаб точно 1.0, сбрасываем перемещение
        if abs(self.zoom_factor - 1.0) < 0.01:
            self.zoom_factor = 1.0
            self.pan_x = 0
            self.pan_y = 0
        
        # Получаем размер виджета изображения
        view_width = self.image_label.width()
        view_height = self.image_label.height()
        
        # Создаем новый QPixmap для отображения (оптимизированное создание)
        final_pixmap = QPixmap(view_width, view_height)
        final_pixmap.fill(Qt.GlobalColor.transparent)
        
        # Масштабируем исходное изображение по размерам окна (оптимизированное масштабирование)
        if self.current_pixmap.width() > view_width or self.current_pixmap.height() > view_height:
            scaled_pixmap = self.current_pixmap.scaled(
                view_width, view_height,
                Qt.AspectRatioMode.KeepAspectRatio, 
                Qt.TransformationMode.SmoothTransformation
            )
        else:
            scaled_pixmap = self.current_pixmap  # Используем оригинал если он меньше
        
        # Получаем базовые координаты изображения (центрирование)
        base_x = (view_width - scaled_pixmap.width()) // 2
        base_y = (view_height - scaled_pixmap.height()) // 2
        
        painter = QPainter(final_pixmap)
        painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform, True)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        
        if self.zoom_factor != 1.0:  # Обрабатываем любой масштаб, отличный от 1.0
            # Размер масштабированного изображения
            zoomed_width = int(scaled_pixmap.width() * self.zoom_factor)
            zoomed_height = int(scaled_pixmap.height() * self.zoom_factor)
            
            # Расчет видимой области с учетом перемещения
            visible_width = min(view_width, zoomed_width)
            visible_height = min(view_height, zoomed_height)
            
            if self.zoom_factor > 1.0:
                # Ограничиваем перемещение только при увеличении
                max_pan_x = max(0, (zoomed_width - view_width) // 2)
                max_pan_y = max(0, (zoomed_height - view_height) // 2)
                self.pan_x = max(-max_pan_x, min(max_pan_x, self.pan_x))
                self.pan_y = max(-max_pan_y, min(max_pan_y, self.pan_y))
            else:
                # При уменьшении сбрасываем перемещение
                self.pan_x = 0
                self.pan_y = 0
            
            # Позиция с учетом перемещения и масштаба
            x_offset = (zoomed_width - view_width) // 2 - int(self.pan_x)
            y_offset = (zoomed_height - view_height) // 2 - int(self.pan_y)
            
            # Обеспечиваем, чтобы offset был неотрицательным
            x_offset = max(0, min(zoomed_width - visible_width, x_offset))
            y_offset = max(0, min(zoomed_height - visible_height, y_offset))
            
            # Исходный прямоугольник (откуда брать пиксели)
            source_rect = QRectF(
                x_offset / self.zoom_factor,
                y_offset / self.zoom_factor,
                visible_width / self.zoom_factor,
                visible_height / self.zoom_factor
            )
            
            # Целевой прямоугольник (куда рисовать)
            target_rect = QRectF(
                (view_width - visible_width) // 2,
                (view_height - visible_height) // 2,
                visible_width,
                visible_height
            )
            
            # Рисуем изображение
            painter.drawPixmap(target_rect, scaled_pixmap, source_rect)
        else:
            # Рисуем обычное изображение (без масштабирования)
            painter.drawPixmap(base_x, base_y, scaled_pixmap)
        
        # Если включена сетка, рисуем её поверх изображения
        if self.show_grid:
            self.draw_grid(painter, view_width, view_height)
        
        painter.end()
        self.image_label.setPixmap(final_pixmap)

    def draw_grid(self, painter, width, height, offset_x=0, offset_y=0):
        """Рисует сетку на изображении"""
        if not self.show_grid:
            return
        
        # Получаем цвет сетки из текущей темы
        colors = theme_manager.get_theme_colors()
        self.grid_color = QColor(colors['grid_color'])
        
        # Рисуем горизонтальные линии
        if self.grid_h_lines > 0:
            # Рассчитываем интервал между линиями
            h_interval = height / (self.grid_h_lines + 1)
            
            for i in range(1, self.grid_h_lines + 1):
                y = int(i * h_interval + offset_y)  # Преобразуем в целое число
                painter.setPen(QPen(self.grid_color, 1))
                painter.drawLine(int(offset_x), y, int(width + offset_x), y)
                
        # Рисуем вертикальные линии
        if self.grid_v_lines > 0:
            # Рассчитываем интервал между линиями
            v_interval = width / (self.grid_v_lines + 1)
            
            for i in range(1, self.grid_v_lines + 1):
                x = int(i * v_interval + offset_x)  # Преобразуем в целое число
                painter.setPen(QPen(self.grid_color, 1))
                painter.drawLine(x, int(offset_y), x, int(height + offset_y))

    def closeEvent(self, event):
        """Обрабатывает закрытие главного окна"""
        # Stop scanner thread if running
        if hasattr(self, 'scanner_thread') and self.scanner_thread is not None and self.scanner_thread.isRunning():
            logging.debug("Requesting scanner thread to stop...")
            self.scanner_thread.stop()

        # Stop all image loaders
        if hasattr(self, 'image_loaders'):
            for loader in list(self.image_loaders.values()):
                if loader is not None:
                    try:
                        loader.quit()
                    except Exception as e:
                        logging.error(f"Error stopping loader: {e}")
            self.image_loaders.clear()
        
        # Stop UI timers
        if hasattr(self, 'countdown_timer'):
            self.countdown_timer.stop()
        if hasattr(self, 'break_timer'):
            self.break_timer.stop()
        if hasattr(self, 'zoom_indicator_timer'):
            self.zoom_indicator_timer.stop()
        if hasattr(self, 'system_theme_check_timer'):
            self.system_theme_check_timer.stop()
        
        # Save history if enabled
        if hasattr(self, 'settings') and hasattr(self, 'displayed_history') and hasattr(self, 'config_manager') and self.settings.get("save_history", True):
            self.config_manager.save_history(self.displayed_history)
        
        logging.debug("Main window closing")
        event.accept()

    def copy_image_to_clipboard(self):
        """Копирует текущее изображение в буфер обмена"""
        if hasattr(self, 'current_pixmap') and self.current_pixmap:
            clipboard = QApplication.clipboard()
            clipboard.setPixmap(self.current_pixmap)
            
            # Получаем цвета темы
            colors = theme_manager.get_theme_colors()
            
            # Показываем всплывающее сообщение об успешном копировании
            status_bar = QStatusBar(self)
            status_bar.setStyleSheet(f"""
                QStatusBar {{
                    background-color: {colors['background_secondary']};
                    color: {colors['text']};
                    border: 1px solid {colors['border']};
                    border-radius: 4px;
                    padding: 4px;
                    font-size: 12px;
                }}
            """)
            status_bar.showMessage(tr("Image copied to clipboard"), 2000)
            status_bar.move(10, self.height() - 40)
            status_bar.setFixedWidth(200)
            status_bar.show()
            
            # Скрываем сообщение через 2 секунды
            QTimer.singleShot(2000, status_bar.hide)
        
    def zoom_in(self):
        """Увеличивает масштаб на 10%"""
        self.zoom_factor *= 1.1
        self.zoom_factor = min(10.0, self.zoom_factor)  # Ограничиваем максимальный зум
        self.update_image_display()
        self.show_zoom_indicator()

    def zoom_out(self):
        """Уменьшает масштаб на 10%"""
        prev_zoom = self.zoom_factor
        self.zoom_factor *= 0.9
        
        # Ограничиваем минимальный масштаб до 0.5 (50%)
        self.zoom_factor = max(0.5, min(10.0, self.zoom_factor))
        
        # Сбрасываем pan при возврате к нормальному масштабу
        if prev_zoom > 1.0 and self.zoom_factor <= 1.0:
            self.zoom_factor = 1.0
            self.pan_x = 0
            self.pan_y = 0
        
        self.update_image_display()
        self.show_zoom_indicator()

    def reset_zoom(self):
        """Сбрасывает масштаб к исходному"""
        self.zoom_factor = 1.0
        self.pan_x = 0
        self.pan_y = 0
        self.update_image_display()
        self.show_zoom_indicator()

    def show_zoom_indicator(self):
        """Показывает индикатор текущего масштаба"""
        zoom_percent = int(self.zoom_factor * 100)
        self.zoom_indicator.setText(f"{zoom_percent}%")
        
        # Позиционируем индикатор в правом нижнем углу
        self.zoom_indicator.adjustSize()
        self.zoom_indicator.move(
            self.width() - self.zoom_indicator.width() - 20,
            self.height() - self.zoom_indicator.height() - 20
        )
        
        self.zoom_indicator.show()
        
        # Сбрасываем и запускаем таймер
        self.zoom_indicator_timer.stop()
        self.zoom_indicator_timer.start(1500)  # Показываем 1.5 секунды

    def apply_theme(self):
        """Применяет текущую тему ко всем элементам интерфейса"""
        # Получаем подходящий шрифт и размеры для текущего языка
        current_language = self.settings.get("language", "en")
        font_family = get_font_family_for_language(current_language)
        font_sizes = get_font_size_for_language(current_language)
        
        styles = theme_manager.get_theme_styles(font_family, font_sizes)
        colors = theme_manager.get_theme_colors()
        
        # Обновляем глобальные стили приложения, включая подсказки
        app = QApplication.instance()
        app.setStyleSheet(f"""
            QWidget {{
                font-family: {font_family};
                font-size: {font_sizes['base']}px;
            }}
            QLabel {{
                font-family: {font_family};
                font-size: {font_sizes['label']}px;
            }}
            QPushButton {{
                font-family: {font_family};
                font-size: {font_sizes['base']}px;
            }}
            QSpinBox {{
                font-family: {font_family};
                font-size: {font_sizes['base']}px;
            }}
            QComboBox {{
                font-family: {font_family};
                font-size: {font_sizes['base']}px;
            }}
            QCheckBox {{
                font-family: {font_family};
                font-size: {font_sizes['base']}px;
            }}
            {styles["tooltip"]}
        """)
        
        # Применяем стиль к главному окну и его центральному виджету
        self.setStyleSheet(styles["main_window"])
        self.central_widget.setStyleSheet(styles["main_window"])
        
        # Применяем стиль к изображению
        self.image_label.setStyleSheet(f"background-color: {colors['background']};")
        
        # Применяем стиль к таймеру
        timer_container = self.timer_label.parent()
        timer_container.setStyleSheet(styles["timer_label"])
        
        # Применяем стиль к индикатору зума
        self.zoom_indicator.setStyleSheet(styles["zoom_indicator"])
        
        # Применяем стиль к кнопкам навигации
        for button in [self.prev_button, self.pause_button, self.skip_button, self.next_button]:
            button.setStyleSheet(styles["nav_buttons"])
        
        # Применяем стиль ко всем кнопкам инструментов, включая кнопку настроек
        for button in [self.open_file_button, self.copy_image_button, self.delete_button,
                       self.grid_button, self.bw_button, self.flip_v_button,
                       self.flip_h_button, self.rotate_button, self.restore_button,
                       self.show_timer_button, self.always_on_top_button,
                       self.settings_button]:
            button.setStyleSheet(styles["tool_buttons"])
            button.setFixedSize(36, 36)
            
        # Применяем стиль к контейнеру кнопок
        for control in self.overlay_controls:
            if isinstance(control, QWidget) and control is not timer_container:
                control.setStyleSheet(styles["buttons_container"])
        
        # Обновляем иконки в соответствии с темой
        self.prev_button.setIcon(create_themed_icon("resources/past.png"))
        self.pause_button.setIcon(create_themed_icon("resources/pause0.png" if not self.is_paused else "resources/play0.png"))
        self.skip_button.setIcon(create_themed_icon("resources/skip0.png"))
        self.next_button.setIcon(create_themed_icon("resources/next.png"))
        self.open_file_button.setIcon(create_themed_icon("resources/file0.png"))
        self.copy_image_button.setIcon(create_themed_icon("resources/copy0.png"))
        self.delete_button.setIcon(create_themed_icon("resources/delete0.png"))
        self.grid_button.setIcon(create_themed_icon("resources/grid1.png" if self.show_grid else "resources/grid0.png"))
        self.bw_button.setIcon(create_themed_icon("resources/bwfilter1.png" if self.is_bw else "resources/bwfilter0.png"))
        self.flip_v_button.setIcon(create_themed_icon("resources/flipv.png"))
        self.flip_h_button.setIcon(create_themed_icon("resources/fliph.png"))
        self.rotate_button.setIcon(create_themed_icon("resources/rotate.png"))
        self.restore_button.setIcon(create_themed_icon("resources/reset.png"))
        self.show_timer_button.setIcon(create_themed_icon("resources/timer1.png" if self.show_timer_button.isChecked() else "resources/timer0.png"))
        self.always_on_top_button.setIcon(create_themed_icon("resources/pin1.png" if self.always_on_top_button.isChecked() else "resources/pin0.png"))
        self.settings_button.setIcon(create_themed_icon("resources/settings0.png"))
        
        # Обновляем цвет сетки
        self.grid_color = QColor(colors['grid_color'])
        
        # Обновляем отображение
        self.update_image_display()

    def showEvent(self, event):
        """Обработка показа главного окна"""
        super().showEvent(event)
        
        # Восстанавливаем правильное состояние видимости таймера
        is_visible = self.settings.get("show_timer", True)
        timer_container = self.timer_label.parent()
        
        if is_visible:
            timer_container.show()
            self.timer_label.show()
        else:
            timer_container.hide()
            self.timer_label.hide()
        
        # Применяем текущую системную тему, если она выбрана
        if self.settings.get("theme") == "system":
            self.check_and_apply_system_theme()

    def changeEvent(self, event):
        """Обработка изменения состояния окна, включая изменения системной темы"""
        if event.type() == QEvent.Type.WindowStateChange:
            if self.windowState() & Qt.WindowState.WindowMinimized:
                # Окно свернуто
                pass
            else:
                # Окно восстановлено
                is_visible = self.settings.get("show_timer", True)
                timer_container = self.timer_label.parent()
                
                if is_visible:
                    timer_container.show()
                    self.timer_label.show()
                else:
                    timer_container.hide()
                    self.timer_label.hide()
                
                # Если окно было восстановлено или развернуто, проверяем системную тему
                if self.settings.get("theme") == "system":
                    self.check_and_apply_system_theme()
        
        # Продолжаем стандартную обработку события
        super().changeEvent(event)

    def check_and_apply_system_theme(self):
        """Проверяет и при необходимости обновляет тему в соответствии с системной"""
        # Получаем текущую системную тему
        system_theme = theme_manager.get_windows_theme()
        # Если текущая тема приложения не соответствует системной, обновляем
        if theme_manager.get_current_theme() != system_theme:
            logging.info(f"Updating theme to match system: {system_theme}")
            theme_manager.set_theme("system")  # Это установит правильную определенную тему
            self.apply_theme()

    def check_system_theme(self):
        """Периодически проверяет системную тему и обновляет при необходимости"""
        if self.settings.get("theme") == "system":
            self.check_and_apply_system_theme()

class ImageLoaderThread(QThread):
    loaded = pyqtSignal(str, QPixmap)
    error = pyqtSignal(str, str)

    def __init__(self, image_path, parent=None):
        super().__init__(parent)
        self.image_path = image_path
        self._is_running = True

    def quit(self):
        self._is_running = False
        super().quit()

    def load_svg(self, path):
        if not self._is_running:
            return None
        try:
            renderer = QSvgRenderer(path)
            if not renderer.isValid():
                return None

            image = QImage(800, 600, QImage.Format.Format_ARGB32)
            image.fill(Qt.GlobalColor.transparent)
            
            painter = None
            try:
                painter = QPainter(image)
                renderer.render(painter)
                return QPixmap.fromImage(image)
            finally:
                if painter:
                    painter.end()
        except Exception as e:
            logging.error(f"Error loading SVG {path}: {e}")
            return None

    def run(self):
        try:
            if not self._is_running:
                return

            if not os.path.exists(self.image_path):
                self.error.emit(self.image_path, "File doesn't exist")
                return

            if not os.access(self.image_path, os.R_OK):
                self.error.emit(self.image_path, "No access to file")
                return

            ext = os.path.splitext(self.image_path)[1].lower()
            
            if not self._is_running:
                return
                
            if ext == '.svg':
                pixmap = self.load_svg(self.image_path)
            else:
                pixmap = QPixmap(self.image_path)

            if not self._is_running:
                return

            if pixmap is None or pixmap.isNull():
                self.error.emit(self.image_path, "Failed to load image")
                return

            self.loaded.emit(self.image_path, pixmap)

        except Exception as e:
            if self._is_running:
                self.error.emit(self.image_path, str(e))

class DeleteConfirmationDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle(tr("Confirm file deletion"))
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Dialog)
        self.setModal(True)
        
        # Создаем layout
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(15)
        
        # Получаем цвета темы
        colors = theme_manager.get_theme_colors()
        
        # Устанавливаем стиль для диалога
        self.setStyleSheet(f"""
            QDialog {{
                background-color: {colors['background']};
                border: 1px solid {colors['border']};
                border-radius: 8px;
            }}
            QLabel {{
                color: {colors['text']};
                font-size: 14px;
            }}
        """)
        
        # Добавляем иконку предупреждения
        icon_label = QLabel()
        warning_icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxWarning)
        icon_label.setPixmap(warning_icon.pixmap(32, 32))
        icon_layout = QHBoxLayout()
        icon_layout.addWidget(icon_label)
        icon_layout.addStretch()
        layout.addLayout(icon_layout)
        
        # Добавляем текст
        message = QLabel(tr("Are you sure you want to move the file to trash?"))
        message.setWordWrap(True)
        message.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(message)
        
        # Кнопки
        button_layout = QHBoxLayout()
        self.no_button = QPushButton(tr("No"))
        self.yes_button = QPushButton(tr("Yes"))
        
        # Стилизация кнопок
        button_style = f"""
            QPushButton {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                border: 1px solid {colors['border']};
                border-radius: 6px;
                padding: 8px 16px;
                min-width: 100px;
            }}
            QPushButton:hover {{
                background-color: {colors['background_hover']};
                border: 1px solid {colors['border_hover']};
            }}
            QPushButton:pressed {{
                background-color: {colors['background_pressed']};
            }}
        """
        
        danger_button_style = f"""
            QPushButton {{
                background-color: #d32f2f;
                color: white;
                border: 1px solid #b71c1c;
                border-radius: 6px;
                padding: 8px 16px;
                min-width: 100px;
            }}
            QPushButton:hover {{
                background-color: #f44336;
                border: 1px solid #d32f2f;
            }}
            QPushButton:pressed {{
                background-color: #b71c1c;
            }}
        """
        
        self.no_button.setStyleSheet(button_style)
        self.yes_button.setStyleSheet(danger_button_style)
        
        button_layout.addWidget(self.no_button)
        button_layout.addWidget(self.yes_button)
        layout.addLayout(button_layout)
        
        # Подключаем сигналы
        self.yes_button.clicked.connect(self.accept)
        self.no_button.clicked.connect(self.reject)
        
        # Устанавливаем фокус на кнопку "Нет" по умолчанию
        self.no_button.setFocus()
        
    def paintEvent(self, event):
        # Отрисовка границы окна
        painter = QPainter(self)
        painter.setPen(QPen(self.palette().color(QPalette.ColorRole.Mid)))
        painter.drawRect(0, 0, self.width() - 1, self.height() - 1)

class NoticeDialog(QDialog):
    def __init__(self, parent=None, title="", message="", icon_type="information"):
        super().__init__(parent)
        self.setWindowTitle(title)
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Dialog)
        self.setModal(True)
        
        # Создаем layout
        self.layout = QVBoxLayout(self)
        self.layout.setContentsMargins(20, 20, 20, 20)
        self.layout.setSpacing(15)
        
        # Добавляем иконку
        self.icon_label = QLabel()
        if icon_type == "information":
            icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxInformation)
        elif icon_type == "warning":
            icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxWarning)
        elif icon_type == "critical":
            icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxCritical)
        elif icon_type == "question":
            icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxQuestion)
        else:
            icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxInformation)
            
        self.icon_label.setPixmap(icon.pixmap(32, 32))
        icon_layout = QHBoxLayout()
        icon_layout.addWidget(self.icon_label)
        icon_layout.addStretch()
        self.layout.addLayout(icon_layout)
        
        # Добавляем текст
        self.message_label = QLabel(message)
        self.message_label.setWordWrap(True)
        self.message_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.layout.addWidget(self.message_label)
        
        # Кнопка OK
        button_layout = QHBoxLayout()
        self.ok_button = QPushButton(tr("OK"))
        button_layout.addStretch()
        button_layout.addWidget(self.ok_button)
        button_layout.addStretch()
        self.layout.addLayout(button_layout)
        
        # Подключаем сигналы
        self.ok_button.clicked.connect(self.accept)
        
        # Устанавливаем фокус на кнопку OK
        self.ok_button.setFocus()
        
        # Центрирование диалога относительно родительского окна
        self.center_on_parent()
        
        # Применяем текущую тему
        self.apply_theme()
        
    def apply_theme(self):
        """Применяет текущую тему к элементам диалога"""
        colors = theme_manager.get_theme_colors()
        
        # Стиль для диалога
        self.setStyleSheet(f"""
            QDialog {{
                background-color: {colors['background']};
                border: 1px solid {colors['border']};
                border-radius: 8px;
            }}
        """)
        
        # Стиль для текста сообщения
        self.message_label.setStyleSheet(f"""
            QLabel {{
                color: {colors['text']};
                font-size: 14px;
                qproperty-wordWrap: true;
            }}
        """)
        
        # Стиль для кнопки
        self.ok_button.setStyleSheet(f"""
            QPushButton {{
                background-color: {colors['background_secondary']};
                color: {colors['text']};
                border: 1px solid {colors['border']};
                border-radius: 6px;
                padding: 8px 16px;
                min-width: 100px;
            }}
            QPushButton:hover {{
                background-color: {colors['background_hover']};
                border: 1px solid {colors['border_hover']};
            }}
            QPushButton:pressed {{
                background-color: {colors['background_pressed']};
            }}
        """)
        
    def center_on_parent(self):
        """Центрирует диалог относительно родительского окна"""
        if self.parent():
            parent_geo = self.parent().geometry()
            # Обходим проблему с указанием self.size() до появления окна
            self.adjustSize()  # Подгоняем размер под содержимое
            self.setFixedSize(max(300, self.width()), self.height())  # Минимальная ширина 300px
            
            x = parent_geo.x() + (parent_geo.width() - self.width()) // 2
            y = parent_geo.y() + (parent_geo.height() - self.height()) // 2
            
            self.move(x, y)
        
    @staticmethod
    def show_information(parent, title, message):
        dialog = NoticeDialog(parent, title, message, "information")
        dialog.exec()
        
    @staticmethod
    def show_warning(parent, title, message):
        dialog = NoticeDialog(parent, title, message, "warning")
        dialog.exec()
        
    @staticmethod
    def show_error(parent, title, message):
        dialog = NoticeDialog(parent, title, message, "critical")
        dialog.exec()
        
    @staticmethod
    def show_question(parent, title, message):
        dialog = NoticeDialog(parent, title, message, "question")
        return dialog.exec() == QDialog.DialogCode.Accepted

def main():
    app = QApplication(sys.argv)
    
    # Загружаем настройки и устанавливаем язык
    config_manager = ConfigManager()
    settings = config_manager.load_settings()
    current_language = settings.get("language", "en")
    translation_manager.set_locale(current_language)  # English by default
    theme_manager.set_theme(settings.get("theme", "dark")) # Устанавливаем тему до стилей
    
    # Получаем подходящий шрифт и размеры для текущего языка
    font_family = get_font_family_for_language(current_language)
    font_sizes = get_font_size_for_language(current_language)
    
    # Устанавливаем базовый шрифт для всего приложения
    font = app.font()
    # Извлекаем первый шрифт из списка fallback
    primary_font = font_family.split(',')[0].strip().strip('"')
    font.setFamily(primary_font)
    font.setPointSize(font_sizes['base'])
    app.setFont(font)
    
    # Получаем цвета текущей темы и стили с правильным шрифтом и размерами
    colors = theme_manager.get_theme_colors()
    styles = theme_manager.get_theme_styles(font_family, font_sizes)
    
    # Устанавливаем стили для всего приложения
    app.setStyleSheet(f"""
        QWidget {{
            font-family: {font_family};
            font-size: {font_sizes['base']}px;
        }}
        QLabel {{
            font-family: {font_family};
            font-size: {font_sizes['label']}px;
        }}
        QPushButton {{
            font-family: {font_family};
            font-size: {font_sizes['base']}px;
        }}
        QSpinBox {{
            font-family: {font_family};
            font-size: {font_sizes['base']}px;
        }}
        QComboBox {{
            font-family: {font_family};
            font-size: {font_sizes['base']}px;
        }}
        QCheckBox {{
            font-family: {font_family};
            font-size: {font_sizes['base']}px;
        }}
        {styles["tooltip"]}
    """)

    window = MainWindow()
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()

