"""Theme management module"""

import importlib
import logging
import sys
from typing import Dict, Optional

class ThemeManager:
    _instance = None
    _current_theme = "dark"
    _available_themes = ["system", "dark", "light", "calcite", "charoite", "emerald", "jasper", "ruby", "sapphire"]
    
    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(ThemeManager, cls).__new__(cls)
        return cls._instance
    
    @classmethod
    def get_instance(cls):
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance
    
    def get_windows_theme(self):
        """Определяет текущую тему Windows (светлая или темная)"""
        if sys.platform != 'win32':
            return "dark"  # По умолчанию для не-Windows систем
        
        try:
            import winreg
            registry = winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER)
            key = winreg.OpenKey(registry, r"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize")
            value, _ = winreg.QueryValueEx(key, "AppsUseLightTheme")
            winreg.CloseKey(key)
            
            # value = 0 означает темную тему, value = 1 означает светлую тему
            return "light" if value == 1 else "dark"
        except Exception as e:
            logging.warning(f"Не удалось определить системную тему Windows: {e}")
            return "dark"  # По умолчанию, если не удалось определить
    
    def set_theme(self, theme):
        """Устанавливает тему оформления. При выборе 'system' определяет системную тему."""
        if theme == "system":
            detected_theme = self.get_windows_theme()
            logging.info(f"Определена системная тема: {detected_theme}")
            self._current_theme = detected_theme
        else:
            self._current_theme = theme
        
    def get_current_theme(self):
        return self._current_theme
    
    def get_theme_colors(self):
        """Получает цвета текущей темы"""
        try:
            theme_module = importlib.import_module(f"themes.{self._current_theme}")
            return theme_module.get_colors()
        except ImportError as e:
            logging.error(f"Не удалось загрузить тему {self._current_theme}: {e}")
            # Возвращаем темную тему как запасной вариант
            fallback_module = importlib.import_module("themes.dark")
            return fallback_module.get_colors()
            
    def get_theme_styles(self, font_family='"Segoe UI", sans-serif', font_sizes=None):
        """Получает стили на основе цветов текущей темы"""
        colors = self.get_theme_colors()
        
        # Размеры шрифтов по умолчанию
        if font_sizes is None:
            font_sizes = {'base': 10, 'label': 14, 'header': 14, 'tooltip': 12}
        return {
            "main_window": f"""
                QMainWindow {{
                    background-color: {colors['background']};
                }}
            """,
            
            "settings_container": f"""
                QWidget#settingsContainer {{
                    background-color: {colors['background']};
                    border-radius: 12px;
                    border: 1px solid {colors['border']};
                }}
                QLabel {{
                    color: {colors['text']};
                    font-size: {font_sizes['label']}px;
                    font-family: {font_family};
                    qproperty-wordWrap: true;
                }}
                QLineEdit, QSpinBox, QComboBox {{
                    background-color: {colors['background_secondary']};
                    border: 1px solid {colors['border']};
                    border-radius: 6px;
                    padding: 6px;
                    color: {colors['text']};
                    font-size: {font_sizes['label']}px;
                    font-family: {font_family};
                    min-height: 20px;
                }}
                QLineEdit:hover, QSpinBox:hover, QComboBox:hover {{
                    background-color: {colors['background_hover']};
                    border: 1px solid {colors['border_hover']};
                }}
                QLineEdit:focus, QSpinBox:focus, QComboBox:focus {{
                    border: 1px solid {colors['border_focus']};
                    background-color: {colors['background_hover']};
                }}
                QPushButton {{
                    background-color: {colors['background_secondary']};
                    color: {colors['text']};
                    border: 1px solid {colors['border']};
                    border-radius: 6px;
                    padding: 8px 16px;
                    font-size: {font_sizes['label']}px;
                    font-family: {font_family};
                    min-width: 100px;
                }}
                QPushButton:hover {{
                    background-color: {colors['background_hover']};
                    border: 1px solid {colors['border_hover']};
                }}
                QPushButton:pressed {{
                    background-color: {colors['background_pressed']};
                }}
                QPushButton:checked {{
                    background-color: {colors['background_checked']};
                    color: {colors['text']};
                }}
            """,
            
            "folder_drop": f"""
                QLabel {{
                    background-color: transparent;
                    border: 2px dashed {colors['border']};
                    border-radius: 8px;
                    padding: 10px;
                    color: {colors['text_secondary']};
                    font-size: 12px;
                }}
                QLabel:hover {{
                    border-color: {colors['border_hover']};
                    color: {colors['text']};
                }}
            """,
            
            "nav_buttons": f"""
                QPushButton {{
                    background-color: {colors['background_secondary']};
                    color: {colors['text']};
                    border: 1px solid {colors['border']};
                    border-radius: 8px;
                    padding: 8px;
                    width: 90px;
                    height: 36px;
                    font-size: 18px;
                    font-weight: bold;
                    margin: 0 5px;
                }}
                QPushButton:hover {{
                    background-color: {colors['background_hover']};
                    border: 1px solid {colors['border_hover']};
                }}
                QPushButton:pressed {{
                    background-color: {colors['background_pressed']};
                }}
                QPushButton:checked {{
                    background-color: {colors['background_checked']};
                }}
            """,
            
            "tool_buttons": f"""
                QPushButton {{
                    background-color: {colors['background_secondary']};
                    color: {colors['text']};
                    border: 1px solid {colors['border']};
                    border-radius: 8px;
                    padding: 0px;
                    width: 36px;
                    height: 36px;
                    font-size: 16px;
                    font-weight: bold;
                    margin: 0 1px;
                }}
                QPushButton:hover {{
                    background-color: {colors['background_hover']};
                    border: 1px solid {colors['border_hover']};
                }}
                QPushButton:pressed {{
                    background-color: {colors['background_pressed']};
                }}
                QPushButton:checked {{
                    background-color: {colors['background_checked']};
                }}
            """,
            
            "buttons_container": f"""
                QWidget {{
                    background-color: {colors['overlay_background']};
                    border-radius: 12px;
                    padding: 10px;
                }}
            """,
            
            "timer_label": f"""
                QWidget {{
                    background-color: {colors['timer_background']};
                    border-radius: 4px;
                    padding: 2px 8px;
                }}
                QLabel {{
                    color: {colors['overlay_text']};
                    font-weight: bold;
                    padding: 2px;
                }}
            """,
            
            "zoom_indicator": f"""
                QLabel {{
                    background-color: {colors['timer_background']};
                    color: {colors['overlay_text']};
                    padding: 4px 8px;
                    border-radius: 4px;
                    font-size: 12px;
                }}
            """,
            
            "notice": f"""
                QLabel {{
                    color: {colors['text']};
                    font-size: {font_sizes['label']}px;
                    qproperty-wordWrap: true;
                }}
            """,
            
            "info_text": f"""
                color: {colors['text']};
                font-size: 14px;
                qproperty-wordWrap: true;
                line-height: 1.4;
            """,
            
            "history_count": f"""
                color: {colors['text_secondary']};
                font-size: 12px;
            """,
            
            "settings_button": f"""
                QPushButton {{
                    background-color: {colors['background_secondary']};
                    color: {colors['text']};
                    border: 1px solid {colors['border']};
                    border-radius: 6px;
                    padding: 8px 16px;
                    font-size: {font_sizes['label']}px;
                    min-width: 100px;
                }}
                QPushButton:hover {{
                    background-color: {colors['background_hover']};
                    border: 1px solid {colors['border_hover']};
                }}
                QPushButton:pressed {{
                    background-color: {colors['background_pressed']};
                }}
            """,
            
            "accent_button": f"""
                QPushButton {{
                    background-color: {colors['accent']};
                    color: {colors['text']};
                    border: none;
                    border-radius: 6px;
                    padding: 8px 16px;
                    font-size: {font_sizes['label']}px;
                    min-width: 100px;
                }}
                QPushButton:hover {{
                    background-color: {colors['accent_hover']};
                }}
                QPushButton:pressed {{
                    background-color: {colors['accent_pressed']};
                }}
            """,
            
            "slider": f"""
                QSlider {{
                    height: 24px;
                }}
                QSlider::groove:horizontal {{
                    border: 1px solid {colors['border']};
                    height: 4px;
                    background: {colors['background_secondary']};
                    margin: 0px;
                    border-radius: 2px;
                }}
                QSlider::handle:horizontal {{
                    background: {colors['accent']};
                    border: none;
                    width: 18px;
                    height: 18px;
                    margin: -7px 0;
                    border-radius: 9px;
                }}
                QSlider::handle:horizontal:hover {{
                    background: {colors['accent_hover']};
                }}
                QSlider::add-page:horizontal {{
                    background: {colors['background_secondary']};
                    border: 1px solid {colors['border']};
                    border-radius: 2px;
                }}
                QSlider::sub-page:horizontal {{
                    background: {colors['accent']};
                    border: 1px solid {colors['border']};
                    border-radius: 2px;
                }}
            """,
            
            "separator": f"""
                QFrame {{
                    color: {colors['separator']};
                }}
            """,
            
            "tooltip": f"""
                QToolTip {{
                    background-color: {colors.get('tooltip_background', colors['overlay_background'])};
                    color: {colors.get('tooltip_text', colors['overlay_text'])};
                    border: 1px solid {colors.get('tooltip_border', colors['border_focus'])};
                    border-radius: 4px;
                    padding: 6px 8px;
                    font-size: {font_sizes['tooltip']}px;
                    font-weight: normal;
                    font-family: {font_family};
                }}
            """
        }

# Создаем глобальный экземпляр менеджера тем
theme_manager = ThemeManager.get_instance()
