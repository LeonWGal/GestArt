import sys
import os
import random
import time
import shutil
import json
from datetime import datetime, timedelta
from pathlib import Path
from functools import partial
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, 
                           QLabel, QPushButton, QFileDialog, QCheckBox, QSpinBox, 
                           QSlider, QComboBox, QFrame, QSizePolicy, QStyle, QMessageBox,
                           QGraphicsOpacityEffect, QDialog)
from PyQt6.QtCore import Qt, QTimer, QSize, QSettings, pyqtSignal, QThread, QDir
from PyQt6.QtGui import QIcon, QPixmap, QImage, QColor, QTransform, QPainter, QFont, QPalette, QAction
from PyQt6.QtCore import QByteArray

class ImageLoader(QThread):
    finished = pyqtSignal(list)
    progress = pyqtSignal(int)
    
    def __init__(self, folder_path):
        super().__init__()
        self.folder_path = folder_path
        
    def run(self):
        images = []
        extensions = ['.jpg', '.jpeg', '.png', '.bmp', '.gif', '.webp']
        total_files = 0
        processed_files = 0
        
        # Сначала подсчитаем общее количество файлов
        for root, _, files in os.walk(self.folder_path):
            for file in files:
                if any(file.lower().endswith(ext) for ext in extensions):
                    total_files += 1
        
        # Теперь загрузим все изображения
        for root, _, files in os.walk(self.folder_path):
            for file in files:
                if any(file.lower().endswith(ext) for ext in extensions):
                    file_path = os.path.join(root, file)
                    images.append(file_path)
                    processed_files += 1
                    self.progress.emit(int(processed_files / total_files * 100))
        
        self.finished.emit(images)

class SettingsWindow(QWidget):
    start_session = pyqtSignal(str, int, int, bool, bool)
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.parent = parent
        self.settings = QSettings("Gestart", "ImageViewer")
        self.init_ui()
        self.load_settings()
        
    def init_ui(self):
        self.setWindowTitle("Gestart - Настройки")
        self.setFixedSize(500, 450)
        
        main_layout = QVBoxLayout()
        main_layout.setSpacing(15)
        main_layout.setContentsMargins(20, 20, 20, 20)
        
        # Стиль для заголовков
        title_style = "QLabel { font-size: 16px; font-weight: bold; color: #5865F2; }"
        
        # Выбор папки
        folder_label = QLabel("Выберите папку с изображениями:")
        folder_label.setStyleSheet(title_style)
        main_layout.addWidget(folder_label)
        
        folder_layout = QHBoxLayout()
        self.folder_path_label = QLabel("Папка не выбрана")
        self.folder_path_label.setStyleSheet("padding: 8px; background-color: #f0f0f0; border-radius: 4px;")
        self.folder_path_label.setMinimumWidth(300)
        
        self.browse_button = QPushButton("Обзор...")
        self.browse_button.setStyleSheet("""
            QPushButton {
                background-color: #5865F2;
                color: white;
                border-radius: 4px;
                padding: 8px 15px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #4752C4;
            }
            QPushButton:pressed {
                background-color: #3C45A5;
            }
        """)
        self.browse_button.clicked.connect(self.browse_folder)
        
        folder_layout.addWidget(self.folder_path_label)
        folder_layout.addWidget(self.browse_button)
        main_layout.addLayout(folder_layout)
        
        # Разделитель
        line = QFrame()
        line.setFrameShape(QFrame.Shape.HLine)
        line.setFrameShadow(QFrame.Shadow.Sunken)
        line.setStyleSheet("background-color: #E3E5E8;")
        main_layout.addWidget(line)
        
        # Время отображения
        time_label = QLabel("Время отображения (секунды):")
        time_label.setStyleSheet(title_style)
        main_layout.addWidget(time_label)
        
        time_layout = QHBoxLayout()
        self.time_spin = QSpinBox()
        self.time_spin.setMinimum(1)
        self.time_spin.setMaximum(3600)
        self.time_spin.setValue(30)
        self.time_spin.setStyleSheet("""
            QSpinBox {
                padding: 8px;
                background-color: #f0f0f0;
                border-radius: 4px;
                min-width: 80px;
            }
        """)
        
        self.infinite_time_check = QCheckBox("Неограниченно")
        self.infinite_time_check.stateChanged.connect(self.toggle_time_spin)
        self.infinite_time_check.setStyleSheet("""
            QCheckBox {
                font-size: 14px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
            }
        """)
        
        time_layout.addWidget(self.time_spin)
        time_layout.addWidget(self.infinite_time_check)
        time_layout.addStretch()
        main_layout.addLayout(time_layout)
        
        # Количество изображений
        img_count_label = QLabel("Количество изображений:")
        img_count_label.setStyleSheet(title_style)
        main_layout.addWidget(img_count_label)
        
        count_layout = QHBoxLayout()
        self.img_count_spin = QSpinBox()
        self.img_count_spin.setMinimum(1)
        self.img_count_spin.setMaximum(9999)
        self.img_count_spin.setValue(10)
        self.img_count_spin.setStyleSheet("""
            QSpinBox {
                padding: 8px;
                background-color: #f0f0f0;
                border-radius: 4px;
                min-width: 80px;
            }
        """)
        
        self.infinite_count_check = QCheckBox("Неограниченно")
        self.infinite_count_check.stateChanged.connect(self.toggle_count_spin)
        self.infinite_count_check.setStyleSheet("""
            QCheckBox {
                font-size: 14px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
            }
        """)
        
        count_layout.addWidget(self.img_count_spin)
        count_layout.addWidget(self.infinite_count_check)
        count_layout.addStretch()
        main_layout.addLayout(count_layout)
        
        # Разделитель
        line2 = QFrame()
        line2.setFrameShape(QFrame.Shape.HLine)
        line2.setFrameShadow(QFrame.Shadow.Sunken)
        line2.setStyleSheet("background-color: #E3E5E8;")
        main_layout.addWidget(line2)
        
        # Дополнительные настройки
        options_label = QLabel("Дополнительные настройки:")
        options_label.setStyleSheet(title_style)
        main_layout.addWidget(options_label)
        
        # История изображений
        self.save_history_check = QCheckBox("Сохранять историю изображений (избегать повторений)")
        self.save_history_check.setStyleSheet("""
            QCheckBox {
                font-size: 14px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
            }
        """)
        main_layout.addWidget(self.save_history_check)
        
        # Кнопка удаления истории
        self.clear_history_button = QPushButton("Удалить историю")
        self.clear_history_button.setStyleSheet("""
            QPushButton {
                background-color: #ED4245;
                color: white;
                border-radius: 4px;
                padding: 8px 15px;
                font-weight: bold;
                max-width: 200px;
            }
            QPushButton:hover {
                background-color: #C03537;
            }
            QPushButton:pressed {
                background-color: #A12D2F;
            }
        """)
        self.clear_history_button.clicked.connect(self.clear_history)
        main_layout.addWidget(self.clear_history_button)
        
        # Перерывы
        self.breaks_check = QCheckBox("Включить перерывы между сессиями")
        self.breaks_check.setStyleSheet("""
            QCheckBox {
                font-size: 14px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
            }
        """)
        main_layout.addWidget(self.breaks_check)
        
        # Позиция таймера
        timer_pos_layout = QHBoxLayout()
        timer_pos_label = QLabel("Расположение таймера:")
        timer_pos_label.setStyleSheet("font-size: 14px;")
        
        self.timer_position = QComboBox()
        self.timer_position.addItems(["Слева", "Справа"])
        self.timer_position.setStyleSheet("""
            QComboBox {
                padding: 8px;
                background-color: #f0f0f0;
                border-radius: 4px;
                min-width: 120px;
            }
        """)
        
        timer_pos_layout.addWidget(timer_pos_label)
        timer_pos_layout.addWidget(self.timer_position)
        timer_pos_layout.addStretch()
        main_layout.addLayout(timer_pos_layout)
        
        # Разделитель
        line3 = QFrame()
        line3.setFrameShape(QFrame.Shape.HLine)
        line3.setFrameShadow(QFrame.Shadow.Sunken)
        line3.setStyleSheet("background-color: #E3E5E8;")
        main_layout.addWidget(line3)
        
        # Начать сессию
        self.start_button = QPushButton("Начать сессию")
        self.start_button.setStyleSheet("""
            QPushButton {
                background-color: #57F287;
                color: white;
                border-radius: 4px;
                padding: 12px;
                font-weight: bold;
                font-size: 16px;
            }
            QPushButton:hover {
                background-color: #45C269;
            }
            QPushButton:pressed {
                background-color: #36A356;
            }
        """)
        self.start_button.clicked.connect(self.start_session_clicked)
        main_layout.addWidget(self.start_button)
        
        self.setLayout(main_layout)
        
        # Установить стиль окна
        self.setStyleSheet("""
            QWidget {
                font-family: 'Segoe UI', Arial, sans-serif;
                font-size: 14px;
                background-color: white;
            }
        """)
    
    def browse_folder(self):
        folder = QFileDialog.getExistingDirectory(self, "Выберите папку с изображениями")
        if folder:
            self.folder_path_label.setText(folder)
            self.settings.setValue("folder_path", folder)
    
    def toggle_time_spin(self, state):
        self.time_spin.setEnabled(not bool(state))
    
    def toggle_count_spin(self, state):
        self.img_count_spin.setEnabled(not bool(state))
    
    def clear_history(self):
        history_file = os.path.join(os.path.expanduser("~"), ".gestart_history.json")
        if os.path.exists(history_file):
            os.remove(history_file)
            QMessageBox.information(self, "Успех", "История просмотра изображений удалена.")
        else:
            QMessageBox.information(self, "Информация", "Истории просмотра не существует.")
    
    def start_session_clicked(self):
        folder_path = self.folder_path_label.text()
        
        if folder_path == "Папка не выбрана":
            QMessageBox.warning(self, "Ошибка", "Пожалуйста, выберите папку с изображениями.")
            return
        
        display_time = 0 if self.infinite_time_check.isChecked() else self.time_spin.value()
        img_count = 0 if self.infinite_count_check.isChecked() else self.img_count_spin.value()
        save_history = self.save_history_check.isChecked()
        use_breaks = self.breaks_check.isChecked()
        
        self.save_settings()
        self.start_session.emit(folder_path, display_time, img_count, save_history, use_breaks)
    
    def save_settings(self):
        self.settings.setValue("display_time", 0 if self.infinite_time_check.isChecked() else self.time_spin.value())
        self.settings.setValue("infinite_time", self.infinite_time_check.isChecked())
        self.settings.setValue("img_count", 0 if self.infinite_count_check.isChecked() else self.img_count_spin.value())
        self.settings.setValue("infinite_count", self.infinite_count_check.isChecked())
        self.settings.setValue("save_history", self.save_history_check.isChecked())
        self.settings.setValue("use_breaks", self.breaks_check.isChecked())
        self.settings.setValue("timer_position", self.timer_position.currentText())
    
    def load_settings(self):
        folder_path = self.settings.value("folder_path", "")
        if folder_path:
            self.folder_path_label.setText(folder_path)
        
        display_time = int(self.settings.value("display_time", 30))
        infinite_time = self.settings.value("infinite_time", False, type=bool)
        
        img_count = int(self.settings.value("img_count", 10))
        infinite_count = self.settings.value("infinite_count", False, type=bool)
        
        save_history = self.settings.value("save_history", False, type=bool)
        use_breaks = self.settings.value("use_breaks", False, type=bool)
        timer_position = self.settings.value("timer_position", "Справа")
        
        self.time_spin.setValue(display_time)
        self.infinite_time_check.setChecked(infinite_time)
        self.time_spin.setEnabled(not infinite_time)
        
        self.img_count_spin.setValue(img_count)
        self.infinite_count_check.setChecked(infinite_count)
        self.img_count_spin.setEnabled(not infinite_count)
        
        self.save_history_check.setChecked(save_history)
        self.breaks_check.setChecked(use_breaks)
        
        index = self.timer_position.findText(timer_position)
        if index >= 0:
            self.timer_position.setCurrentIndex(index)

class TimerDialog(QDialog):
    skip_break = pyqtSignal()
    start_session = pyqtSignal()
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Перерыв между сессиями")
        self.setFixedSize(400, 250)
        self.setWindowFlag(Qt.WindowType.WindowStaysOnTopHint)
        
        layout = QVBoxLayout()
        layout.setSpacing(20)
        layout.setContentsMargins(20, 20, 20, 20)
        
        # Заголовок
        title_label = QLabel("Перерыв между сессиями")
        title_label.setStyleSheet("font-size: 18px; font-weight: bold; color: #5865F2;")
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(title_label)
        
        # Сообщение
        message_label = QLabel("Сделайте небольшой перерыв перед началом следующей сессии.")
        message_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        message_label.setStyleSheet("font-size: 14px;")
        layout.addWidget(message_label)
        
        # Таймер
        self.timer_label = QLabel("00:30")
        self.timer_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.timer_label.setStyleSheet("font-size: 42px; font-weight: bold; color: #5865F2;")
        layout.addWidget(self.timer_label)
        
        # Кнопки
        buttons_layout = QHBoxLayout()
        
        self.skip_button = QPushButton("Пропустить перерыв")
        self.skip_button.setStyleSheet("""
            QPushButton {
                background-color: #ED4245;
                color: white;
                border-radius: 4px;
                padding: 10px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #C03537;
            }
        """)
        self.skip_button.clicked.connect(self.on_skip)
        
        self.start_button = QPushButton("Начать новую сессию")
        self.start_button.setStyleSheet("""
            QPushButton {
                background-color: #57F287;
                color: white;
                border-radius: 4px;
                padding: 10px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #45C269;
            }
        """)
        self.start_button.setEnabled(False)
        self.start_button.clicked.connect(self.on_start)
        
        buttons_layout.addWidget(self.skip_button)
        buttons_layout.addWidget(self.start_button)
        
        layout.addLayout(buttons_layout)
        self.setLayout(layout)
        
        # Таймер
        self.seconds_left = 30
        self.timer = QTimer()
        self.timer.setInterval(1000)
        self.timer.timeout.connect(self.update_timer)
        self.timer.start()
        
        # Стиль для диалога
        self.setStyleSheet("""
            QDialog {
                background-color: white;
                font-family: 'Segoe UI', Arial, sans-serif;
            }
        """)
    
    def update_timer(self):
        self.seconds_left -= 1
        
        if self.seconds_left <= 0:
            self.timer.stop()
            self.timer_label.setText("00:00")
            self.start_button.setEnabled(True)
            self.skip_button.setEnabled(False)
            return
        
        minutes = self.seconds_left // 60
        seconds = self.seconds_left % 60
        self.timer_label.setText(f"{minutes:02d}:{seconds:02d}")
    
    def on_skip(self):
        self.timer.stop()
        self.skip_break.emit()
        self.accept()
    
    def on_start(self):
        self.start_session.emit()
        self.accept()

class ViewerWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.settings = QSettings("Gestart", "ImageViewer")
        self.history = []
        self.current_images = []
        self.current_index = -1
        self.total_images = 0
        self.paused = False
        self.display_time = 30
        self.show_filename = True
        self.show_timer = True
        self.save_history = False
        self.use_breaks = False
        self.timer_position = "Справа"
        self.image_transformations = {}
        self.session_active = False
        
        self.settings_window = SettingsWindow(self)
        self.settings_window.start_session.connect(self.start_new_session)
        
        self.init_ui()
        self.load_history()
        
        # Показываем окно настроек при запуске
        self.settings_window.show()
    
    def init_ui(self):
        self.setWindowTitle("Gestart - Просмотр изображений")
        self.resize(1200, 800)
        
        # Центральный виджет
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        
        # Верхняя панель с таймером и кнопкой "Всегда сверху"
        self.top_bar = QWidget()
        self.top_bar.setObjectName("topBar")
        self.top_bar.setStyleSheet("""
            #topBar {
                background-color: #36393F;
                border-bottom: 1px solid #202225;
            }
        """)
        top_layout = QHBoxLayout(self.top_bar)
        top_layout.setContentsMargins(10, 5, 10, 5)
        
        # Таймер
        self.timer_label = QLabel("00:30")
        self.timer_label.setStyleSheet("""
            QLabel {
                color: white;
                font-size: 16px;
                font-weight: bold;
            }
        """)
        
        # Кнопка "Всегда сверху"
        self.always_on_top_button = QPushButton("Всегда сверху")
        self.always_on_top_button.setCheckable(True)
        self.always_on_top_button.setStyleSheet("""
            QPushButton {
                background-color: #4F545C;
                color: white;
                border-radius: 4px;
                padding: 5px 10px;
            }
            QPushButton:checked {
                background-color: #5865F2;
            }
            QPushButton:hover {
                background-color: #5865F2;
            }
        """)
        self.always_on_top_button.clicked.connect(self.toggle_always_on_top)
        
        # Добавляем виджеты в верхнюю панель в зависимости от настроек
        if self.timer_position == "Слева":
            top_layout.addWidget(self.timer_label)
            top_layout.addStretch()
            top_layout.addWidget(self.always_on_top_button)
        else:
            top_layout.addWidget(self.always_on_top_button)
            top_layout.addStretch()
            top_layout.addWidget(self.timer_label)
        
        main_layout.addWidget(self.top_bar)
        
        # Область для изображения
        self.image_container = QWidget()
        self.image_container.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.image_container.setStyleSheet("background-color: #2F3136;")
        
        image_layout = QVBoxLayout(self.image_container)
        image_layout.setContentsMargins(0, 0, 0, 0)
        
        # Метка для отображения имени файла
        self.filename_label = QLabel()
        self.filename_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.filename_label.setStyleSheet("""
            QLabel {
                color: white;
                font-size: 14px;
                background-color: rgba(32, 34, 37, 150);
                padding: 5px;
            }
        """)
        image_layout.addWidget(self.filename_label)
        
        # Метка для изображения
        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.image_label.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        image_layout.addWidget(self.image_label)
        
        main_layout.addWidget(self.image_container, 1)
        
        # Нижняя панель с элементами управления
        controls_widget = QWidget()
        controls_widget.setObjectName("controlsBar")
        controls_widget.setStyleSheet("""
            #controlsBar {
                background-color: #36393F;
                border-top: 1px solid #202225;
            }
        """)
        controls_layout = QVBoxLayout(controls_widget)
        controls_layout.setContentsMargins(10, 10, 10, 10)
        controls_layout.setSpacing(10)
        
        # Первый ряд элементов управления
        nav_layout = QHBoxLayout()
        
        self.prev_button = QPushButton()
        self.prev_button.setIcon(self.style().standardIcon(QStyle.StandardPixmap.SP_MediaSkipBackward))
        self.prev_button.setToolTip("Предыдущее изображение")
        self.prev_button.clicked.connect(self.show_previous_image)
        
        self.pause_button = QPushButton()
        self.pause_button.setIcon(self.style().standardIcon(QStyle.StandardPixmap.SP_MediaPause))
        self.pause_button.setToolTip("Пауза/Продолжить")
        self.pause_button.clicked.connect(self.toggle_pause)
        
        self.skip_button = QPushButton()
        self.skip_button.setIcon(self.style().standardIcon(QStyle.StandardPixmap.SP_MediaSkipForward))
        self.skip_button.setToolTip("Пропустить")
        self.skip_button.clicked.connect(self.skip_image)
        
        self.next_button = QPushButton()
        self.next_button.setIcon(self.style().standardIcon(QStyle.StandardPixmap.SP_MediaSeekForward))
        self.next_button.setToolTip("Следующее изображение")
        self.next_button.clicked.connect(self.show_next_image)
        
        # Стиль для кнопок навигации
        button_style = """
            QPushButton {
                background-color: #4F545C;
                color: white;
                border-radius: 20px;
                min-width: 40px;
                min-height: 40px;
                padding: 5px;
            }
            QPushButton:hover {
                background-color: #5865F2;
            }
            QPushButton:pressed {
                background-color: #4752C4;
            }
        """
        self.prev_button.setStyleSheet(button_style)
        self.pause_button.setStyleSheet(button_style)
        self.skip_button.setStyleSheet(button_style)
        self.next_button.setStyleSheet(button_style)
        
        nav_layout.addStretch()
        nav_layout.addWidget(self.prev_button)
        nav_layout.addWidget(self.pause_button)
        nav_layout.addWidget(self.skip_button)
        nav_layout.addWidget(self.next_button)
        nav_layout.addStretch()
        
        controls_layout.addLayout(nav_layout)
        
        # Второй ряд элементов управления
        tools_layout = QHBoxLayout()
        
        # Кнопки для действий с изображениями
        tools_style = """
            QPushButton {
                background-color: #4F545C;
                color: white;
                border-radius: 4px;
                padding: 6px 10px;
            }
            QPushButton:hover {
                background-color: #5865F2;
            }
            QPushButton:pressed {
                background-color: #4752C4;
            }
            QPushButton:checked {
                background-color: #5865F2;
            }
        """
        
        self.open_file_button = QPushButton("Открыть файл")
        self.open_file_button.setStyleSheet(tools_style)
        self.open_file_button.clicked.connect(self.open_current_file)
        
        self.delete_file_button = QPushButton("Удалить в корзину")
        self.delete_file_button.setStyleSheet(tools_style)
        self.delete_file_button.clicked.connect(self.delete_current_file)
        
        self.grayscale_button = QPushButton("Ч/Б фильтр")
        self.grayscale_button.setStyleSheet(tools_style)
        self.grayscale_button.setCheckable(True)
        self.grayscale_button.clicked.connect(self.toggle_grayscale)
        
        self.flip_v_button = QPushButton("⇅")
        self.flip_v_button.setToolTip("Отразить по вертикали")
        self.flip_v_button.setStyleSheet(tools_style)
        self.flip_v_button.setCheckable(True)
        self.flip_v_button.clicked.connect(self.toggle_flip_vertical)
        
        self.flip_h_button = QPushButton("⇄")
        self.flip_h_button.setToolTip("Отразить по горизонтали")
        self.flip_h_button.setStyleSheet(tools_style)
        self.flip_h_button.setCheckable(True)
        self.flip_h_button.clicked.connect(self.toggle_flip_horizontal)
        
        self.rotate_button = QPushButton("↻")
        self.rotate_button.setToolTip("Повернуть на 90°")
        self.rotate_button.setStyleSheet(tools_style)
        self.rotate_button.clicked.connect(self.rotate_image)
        
        self.reset_button = QPushButton("Сбросить")
        self.reset_button.setToolTip("Вернуть исходное состояние")
        self.reset_button.setStyleSheet(tools_style)
        self.reset_button.clicked.connect(self.reset_image)
        
        self.show_filename_button = QPushButton("Показать имя файла")
        self.show_filename_button.setStyleSheet(tools_style)
        self.show_filename_button.setCheckable(True)
        self.show_filename_button.setChecked(self.show_filename)
        self.show_filename_button.clicked.connect(self.toggle_filename)
        
        self.show_timer_button = QPushButton("Показать таймер")
        self.show_timer_button.setStyleSheet(tools_style)
        self.show_timer_button.setCheckable(True)
        self.show_timer_button.setChecked(self.show_timer)
        self.show_timer_button.clicked.connect(self.toggle_timer)
        
        self.settings_button = QPushButton("Настройки")
        self.settings_button.setStyleSheet("""
            QPushButton {
                background-color: #5865F2;
                color: white;
                border-radius: 4px;
                padding: 6px 10px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #4752C4;
            }
            QPushButton:pressed {
                background-color: #3C45A5;
            }
        """)
        self.settings_button.clicked.connect(self.show_settings)
        
        tools_layout.addWidget(self.open_file_button)
        tools_layout.addWidget(self.delete_file_button)
        tools_layout.addWidget(self.grayscale_button)
        tools_layout.addWidget(self.flip_v_button)
        tools_layout.addWidget(self.flip_h_button)
        tools_layout.addWidget(self.rotate_button)
        tools_layout.addWidget(self.reset_button)
        tools_layout.addWidget(self.show_filename_button)
        tools_layout.addWidget(self.show_timer_button)
        tools_layout.addWidget(self.settings_button)
        
        controls_layout.addLayout(tools_layout)
        
        main_layout.addWidget(controls_widget)
        
        central_widget.setLayout(main_layout)
        
        # Статус-бар для отображения информации
        self.statusBar().setStyleSheet("""
            QStatusBar {
                background-color: #36393F;
                color: white;
                border-top: 1px solid #202225;
            }
        """)
        
        # Таймер для смены изображений
        self.image_timer = QTimer()
        self.image_timer.timeout.connect(self.show_next_image)
        
        # Таймер обратного отсчета
        self.countdown_timer = QTimer()
        self.countdown_timer.setInterval(1000)
        self.countdown_timer.timeout.connect(self.update_countdown)
        
        # Устанавливаем стиль для всего окна
        self.setStyleSheet("""
            QMainWindow {
                background-color: #36393F;
                font-family: 'Segoe UI', Arial, sans-serif;
            }
        """)
        
        # Инициализируем элементы управления как неактивные
        self.set_controls_enabled(False)
    
    def load_history(self):
        history_file = os.path.join(os.path.expanduser("~"), ".gestart_history.json")
        if os.path.exists(history_file):
            try:
                with open(history_file, 'r') as f:
                    self.history = json.load(f)
            except Exception as e:
                print(f"Error loading history: {e}")
    
    def save_history(self):
        if not self.save_history:
            return
            
        history_file = os.path.join(os.path.expanduser("~"), ".gestart_history.json")
        try:
            with open(history_file, 'w') as f:
                json.dump(self.history, f)
        except Exception as e:
            print(f"Error saving history: {e}")
    
    def toggle_always_on_top(self, checked):
        if checked:
            self.setWindowFlags(self.windowFlags() | Qt.WindowType.WindowStaysOnTopHint)
        else:
            self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowStaysOnTopHint)
        self.show()
    
    def show_settings(self):
        # Делаем настройки полупрозрачными поверх текущего окна
        if not hasattr(self, 'settings_overlay'):
            self.settings_overlay = self.settings_window
            self.settings_overlay.setWindowFlags(Qt.WindowType.Dialog | Qt.WindowType.FramelessWindowHint)
            self.settings_overlay.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
            
            # Создаем полупрозрачный фон
            effect = QGraphicsOpacityEffect(self.settings_overlay)
            effect.setOpacity(0.95)
            self.settings_overlay.setGraphicsEffect(effect)
            
            # Центрируем диалог в родительском окне
            self.settings_overlay.setGeometry(
                self.geometry().center().x() - self.settings_overlay.width() // 2,
                self.geometry().center().y() - self.settings_overlay.height() // 2,
                self.settings_overlay.width(),
                self.settings_overlay.height()
            )
        
        # Останавливаем таймеры
        if self.session_active:
            self.image_timer.stop()
            self.countdown_timer.stop()
            self.paused = True
            self.pause_button.setIcon(self.style().standardIcon(QStyle.StandardPixmap.SP_MediaPlay))
        
        self.settings_overlay.show()
    
    def start_new_session(self, folder_path, display_time, img_count, save_history, use_breaks):
        self.folder_path = folder_path
        self.display_time = display_time
        self.total_images = img_count
        self.save_history = save_history
        self.use_breaks = use_breaks
        
        # Загружаем настройки таймера
        self.timer_position = self.settings.value("timer_position", "Справа")
        
        # Обновляем положение таймера в интерфейсе
        top_layout = self.top_bar.layout()
        while top_layout.count():
            item = top_layout.takeAt(0)
            if item.widget():
                item.widget().setParent(None)
        
        if self.timer_position == "Слева":
            top_layout.addWidget(self.timer_label)
            top_layout.addStretch()
            top_layout.addWidget(self.always_on_top_button)
        else:
            top_layout.addWidget(self.always_on_top_button)
            top_layout.addStretch()
            top_layout.addWidget(self.timer_label)
        
        # Обновляем статус
        self.statusBar().showMessage("Загрузка изображений...")
        
        # Запускаем загрузку изображений в отдельном потоке
        self.image_loader = ImageLoader(folder_path)
        self.image_loader.progress.connect(self.update_load_progress)
        self.image_loader.finished.connect(self.on_images_loaded)
        self.image_loader.start()
        
        # Скрываем окно настроек
        self.settings_window.hide()
        self.showMaximized()
    
    def update_load_progress(self, progress):
        self.statusBar().showMessage(f"Загрузка изображений... {progress}%")
    
    def on_images_loaded(self, images):
        if not images:
            QMessageBox.warning(self, "Ошибка", "В выбранной папке не найдено изображений.")
            self.settings_window.show()
            return
        
        # Убираем из списка изображения, которые уже были просмотрены (если включена история)
        self.all_images = images
        self.current_images = []
        
        if self.save_history:
            for img in images:
                if img not in self.history:
                    self.current_images.append(img)
            
            # Если все изображения уже просмотрены, начинаем заново
            if not self.current_images:
                self.current_images = images.copy()
                self.history = []
        else:
            self.current_images = images.copy()
        
        # Перемешиваем изображения
        random.shuffle(self.current_images)
        
        # Ограничиваем количество изображений, если задано
        if self.total_images > 0 and len(self.current_images) > self.total_images:
            self.current_images = self.current_images[:self.total_images]
        
        self.current_index = -1
        self.image_transformations = {}
        self.session_active = True
        
        # Обновляем статус
        self.statusBar().showMessage(f"Загружено {len(self.current_images)} изображений")
        
        # Активируем элементы управления
        self.set_controls_enabled(True)
        
        # Показываем первое изображение
        self.show_next_image()
    
    def set_controls_enabled(self, enabled):
        self.prev_button.setEnabled(enabled)
        self.pause_button.setEnabled(enabled)
        self.skip_button.setEnabled(enabled)
        self.next_button.setEnabled(enabled)
        self.open_file_button.setEnabled(enabled)
        self.delete_file_button.setEnabled(enabled)
        self.grayscale_button.setEnabled(enabled)
        self.flip_v_button.setEnabled(enabled)
        self.flip_h_button.setEnabled(enabled)
        self.rotate_button.setEnabled(enabled)
        self.reset_button.setEnabled(enabled)
        self.show_filename_button.setEnabled(enabled)
        self.show_timer_button.setEnabled(enabled)
    
    def show_next_image(self):
        if not self.current_images:
            return
        
        # Если это последнее изображение, заканчиваем сессию
        if self.current_index + 1 >= len(self.current_images):
            self.end_session()
            return
        
        self.current_index += 1
        self.display_current_image()
        
        # Добавляем в историю
        current_path = self.current_images[self.current_index]
        if current_path not in self.history and self.save_history:
            self.history.append(current_path)
            self.save_history()
        
        # Запускаем таймер для следующего изображения
        if self.display_time > 0 and not self.paused:
            self.seconds_left = self.display_time
            self.update_timer_label()
            self.image_timer.start(self.display_time * 1000)
            self.countdown_timer.start()
        
        # Обновляем статус
        self.statusBar().showMessage(f"Изображение {self.current_index + 1} из {len(self.current_images)}")
    
    def show_previous_image(self):
        if not self.current_images or self.current_index <= 0:
            return
        
        self.current_index -= 1
        self.display_current_image()
        
        # Запускаем таймер для следующего изображения
        if self.display_time > 0 and not self.paused:
            self.seconds_left = self.display_time
            self.update_timer_label()
            self.image_timer.start(self.display_time * 1000)
            self.countdown_timer.start()
        
        # Обновляем статус
        self.statusBar().showMessage(f"Изображение {self.current_index + 1} из {len(self.current_images)}")
    
    def skip_image(self):
        # Останавливаем текущие таймеры
        self.image_timer.stop()
        self.countdown_timer.stop()
        
        if not self.current_images:
            return
        
        # Удаляем текущее изображение из списка и не увеличиваем индекс
        if self.current_index >= 0 and self.current_index < len(self.current_images):
            self.current_images.pop(self.current_index)
            
            # Если это было последнее изображение, заканчиваем сессию
            if not self.current_images or self.current_index >= len(self.current_images):
                self.end_session()
                return
                
            self.display_current_image()
            
            # Запускаем таймер для следующего изображения
            if self.display_time > 0 and not self.paused:
                self.seconds_left = self.display_time
                self.update_timer_label()
                self.image_timer.start(self.display_time * 1000)
                self.countdown_timer.start()
            
            # Обновляем статус
            self.statusBar().showMessage(f"Изображение {self.current_index + 1} из {len(self.current_images)}")
    
    def toggle_pause(self):
        if not self.session_active:
            return
            
        self.paused = not self.paused
        
        if self.paused:
            self.image_timer.stop()
            self.countdown_timer.stop()
            self.pause_button.setIcon(self.style().standardIcon(QStyle.StandardPixmap.SP_MediaPlay))
        else:
            if self.display_time > 0:
                self.seconds_left = self.display_time
                self.update_timer_label()
                self.image_timer.start(self.display_time * 1000)
                self.countdown_timer.start()
            self.pause_button.setIcon(self.style().standardIcon(QStyle.StandardPixmap.SP_MediaPause))
    
    def update_countdown(self):
        if self.seconds_left > 0:
            self.seconds_left -= 1
            self.update_timer_label()
    
    def update_timer_label(self):
        if not self.show_timer:
            self.timer_label.setText("")
            return
            
        if self.display_time == 0:
            self.timer_label.setText("∞")
        else:
            minutes = self.seconds_left // 60
            seconds = self.seconds_left % 60
            self.timer_label.setText(f"{minutes:02d}:{seconds:02d}")
    
    def toggle_timer(self):
        self.show_timer = not self.show_timer
        self.update_timer_label()
    
    def toggle_filename(self):
        self.show_filename = not self.show_filename
        if self.current_index >= 0 and self.current_index < len(self.current_images):
            if self.show_filename:
                filename = os.path.basename(self.current_images[self.current_index])
                self.filename_label.setText(filename)
            else:
                self.filename_label.setText("")
    
    def end_session(self):
        self.image_timer.stop()
        self.countdown_timer.stop()
        self.session_active = False
        self.image_label.clear()
        self.filename_label.setText("")
        self.timer_label.setText("")
        
        # Обновляем статус
        self.statusBar().showMessage("Сессия завершена")
        
        # Деактивируем элементы управления
        self.set_controls_enabled(False)
        
        # Если включены перерывы, показываем диалог с таймером
        if self.use_breaks:
            timer_dialog = TimerDialog(self)
            timer_dialog.skip_break.connect(self.on_skip_break)
            timer_dialog.start_session.connect(self.on_start_new_session)
            timer_dialog.exec()
        else:
            # Показываем диалог с кнопкой начать новую сессию
            reply = QMessageBox.question(
                self, 
                "Сессия завершена", 
                "Хотите начать новую сессию?",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.Yes
            )
            
            if reply == QMessageBox.StandardButton.Yes:
                self.show_settings()
    
    def on_skip_break(self):
        self.show_settings()
    
    def on_start_new_session(self):
        self.show_settings()
    
    def display_current_image(self):
        if self.current_index < 0 or self.current_index >= len(self.current_images):
            return
            
        current_path = self.current_images[self.current_index]
        
        # Сбрасываем состояние кнопок трансформации
        self.grayscale_button.setChecked(False)
        self.flip_v_button.setChecked(False)
        self.flip_h_button.setChecked(False)
        
        # Загружаем изображение
        pixmap = QPixmap(current_path)
        if pixmap.isNull():
            return
            
        # Применяем сохраненные трансформации, если они есть
        if current_path in self.image_transformations:
            transform = self.image_transformations[current_path]
            if 'grayscale' in transform and transform['grayscale']:
                self.apply_grayscale(pixmap)
                self.grayscale_button.setChecked(True)
            
            if 'flip_v' in transform and transform['flip_v']:
                pixmap = pixmap.transformed(QTransform().scale(1, -1))
                self.flip_v_button.setChecked(True)
            
            if 'flip_h' in transform and transform['flip_h']:
                pixmap = pixmap.transformed(QTransform().scale(-1, 1))
                self.flip_h_button.setChecked(True)
            
            if 'rotation' in transform:
                for _ in range(transform['rotation']):
                    pixmap = pixmap.transformed(QTransform().rotate(90))
        
        # Масштабируем изображение, чтобы оно поместилось в контейнер
        scaled_pixmap = self.scale_pixmap(pixmap)
        self.image_label.setPixmap(scaled_pixmap)
        
        # Обновляем имя файла
        if self.show_filename:
            filename = os.path.basename(current_path)
            self.filename_label.setText(filename)
        else:
            self.filename_label.setText("")
    
    def scale_pixmap(self, pixmap):
        if pixmap.isNull():
            return pixmap
            
        # Получаем размеры контейнера
        container_width = self.image_container.width()
        container_height = self.image_container.height() - self.filename_label.height()
        
        # Масштабируем изображение, сохраняя пропорции
        return pixmap.scaled(
            container_width, 
            container_height, 
            Qt.AspectRatioMode.KeepAspectRatio, 
            Qt.TransformationMode.SmoothTransformation
        )
    
    def resizeEvent(self, event):
        super().resizeEvent(event)
        # При изменении размера окна обновляем изображение
        if hasattr(self, 'current_index') and self.current_index >= 0:
            self.display_current_image()
    
    def apply_grayscale(self, pixmap):
        # Конвертируем QPixmap в QImage для обработки
        image = pixmap.toImage()
        
        for y in range(image.height()):
            for x in range(image.width()):
                pixel = image.pixel(x, y)
                color = QColor(pixel)
                gray = (color.red() + color.green() + color.blue()) // 3
                image.setPixel(x, y, QColor(gray, gray, gray).rgb())
        
        # Обновляем pixmap
        pixmap.convertFromImage(image)
        return pixmap
    
    def toggle_grayscale(self, checked):
        if self.current_index < 0 or self.current_index >= len(self.current_images):
            return
            
        current_path = self.current_images[self.current_index]
        
        # Сохраняем состояние трансформации
        if current_path not in self.image_transformations:
            self.image_transformations[current_path] = {}
        
        self.image_transformations[current_path]['grayscale'] = checked
        
        # Обновляем изображение
        self.display_current_image()
    
    def toggle_flip_vertical(self, checked):
        if self.current_index < 0 or self.current_index >= len(self.current_images):
            return
            
        current_path = self.current_images[self.current_index]
        
        # Сохраняем состояние трансформации
        if current_path not in self.image_transformations:
            self.image_transformations[current_path] = {}
        
        self.image_transformations[current_path]['flip_v'] = checked
        
        # Обновляем изображение
        self.display_current_image()
    
    def toggle_flip_horizontal(self, checked):
        if self.current_index < 0 or self.current_index >= len(self.current_images):
            return
            
        current_path = self.current_images[self.current_index]
        
        # Сохраняем состояние трансформации
        if current_path not in self.image_transformations:
            self.image_transformations[current_path] = {}
        
        self.image_transformations[current_path]['flip_h'] = checked
        
        # Обновляем изображение
        self.display_current_image()
    
    def rotate_image(self):
        if self.current_index < 0 or self.current_index >= len(self.current_images):
            return
            
        current_path = self.current_images[self.current_index]
        
        # Сохраняем состояние трансформации
        if current_path not in self.image_transformations:
            self.image_transformations[current_path] = {'rotation': 0}
        
        if 'rotation' not in self.image_transformations[current_path]:
            self.image_transformations[current_path]['rotation'] = 0
        
        # Увеличиваем счетчик поворотов (по 90 градусов)
        self.image_transformations[current_path]['rotation'] = (
            self.image_transformations[current_path]['rotation'] + 1
        ) % 4
        
        # Обновляем изображение
        self.display_current_image()
    
    def reset_image(self):
        if self.current_index < 0 or self.current_index >= len(self.current_images):
            return
            
        current_path = self.current_images[self.current_index]
        
        # Сбрасываем все трансформации
        if current_path in self.image_transformations:
            del self.image_transformations[current_path]
        
        # Сбрасываем кнопки
        self.grayscale_button.setChecked(False)
        self.flip_v_button.setChecked(False)
        self.flip_h_button.setChecked(False)
        
        # Обновляем изображение
        self.display_current_image()
    
    def open_current_file(self):
        if self.current_index < 0 or self.current_index >= len(self.current_images):
            return
            
        current_path = self.current_images[self.current_index]
        
        # Открываем файл в стандартном просмотрщике изображений
        if sys.platform == 'win32':
            os.startfile(current_path)
        elif sys.platform == 'darwin':  # macOS
            os.system(f'open "{current_path}"')
        else:  # Linux
            os.system(f'xdg-open "{current_path}"')
    
    def delete_current_file(self):
        if self.current_index < 0 or self.current_index >= len(self.current_images):
            return
            
        current_path = self.current_images[self.current_index]
        
        # Подтверждение удаления
        reply = QMessageBox.question(
            self, 
            "Подтверждение удаления", 
            f"Вы уверены, что хотите удалить файл '{os.path.basename(current_path)}'?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No
        )
        
        if reply == QMessageBox.StandardButton.Yes:
            try:
                # Удаляем файл в корзину
                if sys.platform == 'win32':
                    import winshell
                    winshell.delete_file(current_path)
                else:
                    trash_path = os.path.expanduser("~/.local/share/Trash/files/")
                    if not os.path.exists(trash_path):
                        os.makedirs(trash_path)
                    shutil.move(current_path, trash_path)
                
                # Удаляем из истории, если есть
                if current_path in self.history:
                    self.history.remove(current_path)
                    self.save_history()
                
                # Удаляем из списка текущих изображений и показываем следующее
                self.current_images.pop(self.current_index)
                if not self.current_images:
                    self.end_session()
                    return
                elif self.current_index >= len(self.current_images):
                    self.current_index = len(self.current_images) - 1
                
                self.display_current_image()
                self.statusBar().showMessage(f"Файл удален. Изображение {self.current_index + 1} из {len(self.current_images)}")
                
            except Exception as e:
                QMessageBox.warning(self, "Ошибка удаления", f"Не удалось удалить файл: {str(e)}")

def main():
    app = QApplication(sys.argv)
    app.setStyle('Fusion')  # Используем стиль Fusion для современного вида
    
    # Устанавливаем стиль приложения в цветах Discord
    app.setStyleSheet("""
        QToolTip {
            background-color: #18191C;
            color: white;
            border: 1px solid #2F3136;
            padding: 5px;
        }
        
        QScrollBar:vertical {
            border: none;
            background: #2F3136;
            width: 10px;
            margin: 0px;
        }
        
        QScrollBar::handle:vertical {
            background: #202225;
            min-height: 20px;
            border-radius: 5px;
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        
        QMessageBox {
            background-color: #36393F;
            color: white;
        }
        
        QMessageBox QLabel {
            color: white;
        }
        
        QMessageBox QPushButton {
            background-color: #5865F2;
            color: white;
            border-radius: 4px;
            padding: 5px 10px;
            min-width: 80px;
        }
        
        QMessageBox QPushButton:hover {
            background-color: #4752C4;
        }
        
        QLabel {
            color: white;
        }
    """)
    
    viewer = ViewerWindow()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
