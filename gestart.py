import sys
import os
import random
from functools import partial

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QLabel, QPushButton, QWidget,
    QFileDialog, QHBoxLayout, QVBoxLayout, QCheckBox, QGroupBox,
    QSlider, QLineEdit, QSizePolicy, QProgressBar, QToolTip
)
from PyQt6.QtCore import QTimer, Qt, QObject, pyqtSignal, QThread, QEvent, QPointF, QPropertyAnimation, QRectF
from PyQt6.QtGui import QPixmap, QTransform, QPainter, QShortcut, QKeySequence, QColor, QFont

from send2trash import send2trash

# Класс для асинхронной загрузки изображений
class ImageLoaderWorker(QObject):
    finished = pyqtSignal(list)

    def load_folder(self, folder):
        valid_extensions = ('.jpg', '.jpeg', '.png', '.bmp', '.gif')
        file_list = [
            os.path.join(root, file)
            for root, _, files in os.walk(folder)
            for file in files
            if file.lower().endswith(valid_extensions)
        ]
        self.finished.emit(file_list)

# QLabel с поддержкой зумирования и панорамирования
class ZoomableLabel(QLabel):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._dragging = False
        self._last_pos = None
        self.zoom_factor = 1.0
        self.min_zoom = 0.1
        self.max_zoom = 5.0
        self._base_pixmap = None
        self.offset = QPointF(0, 0)
        self.setStyleSheet("background-color: #21252b; border-radius: 12px;")
        self.setMinimumSize(200, 200)

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self._dragging = True
            self._last_pos = event.position()

    def mouseMoveEvent(self, event):
        if self._dragging and self._base_pixmap:
            delta = event.position() - self._last_pos
            self.offset += delta
            self._last_pos = event.position()
            self.update()

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self._dragging = False

    def setBasePixmap(self, pixmap):
        self._base_pixmap = pixmap
        self.resetZoom()

    def resetZoom(self):
        self.zoom_factor = 1.0
        if self._base_pixmap:
            base_size = self._base_pixmap.size().scaled(self.size(), Qt.AspectRatioMode.KeepAspectRatio)
            target_width = int(base_size.width() * self.zoom_factor)
            target_height = int(base_size.height() * self.zoom_factor)
            self.offset = QPointF((self.width() - target_width) / 2, (self.height() - target_height) / 2)
        self.update()

    def wheelEvent(self, event):
        if QApplication.keyboardModifiers() == Qt.KeyboardModifier.ControlModifier and self._base_pixmap:
            pos = event.position()
            old_zoom = self.zoom_factor
            new_zoom = old_zoom * 1.1 if event.angleDelta().y() > 0 else old_zoom / 1.1
            new_zoom = max(self.min_zoom, min(self.max_zoom, new_zoom))
            ratio = new_zoom / old_zoom
            self.offset = pos - (pos - self.offset) * ratio
            self.zoom_factor = new_zoom
            self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        if self._base_pixmap:
            base_size = self._base_pixmap.size().scaled(self.size(), Qt.AspectRatioMode.KeepAspectRatio)
            target_width = int(base_size.width() * self.zoom_factor)
            target_height = int(base_size.height() * self.zoom_factor)
            scaled_pixmap = self._base_pixmap.scaled(
                target_width, target_height,
                Qt.AspectRatioMode.KeepAspectRatio,
                Qt.TransformationMode.SmoothTransformation
            )
            painter.drawPixmap(int(self.offset.x()), int(self.offset.y()), scaled_pixmap)
        else:
            painter.fillRect(self.rect(), QColor("#21252b"))
            painter.setPen(QColor("#99aab5"))
            painter.drawText(self.rect(), Qt.AlignmentFlag.AlignCenter, "Выберите папку с изображениями")

    def resizeEvent(self, event):
        self.resetZoom() if self._base_pixmap else super().resizeEvent(event)

# Основное приложение
class GesturePosesApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("GestureArt")
        self.setGeometry(100, 100, 1200, 800)
        self.setStyleSheet("""
            QMainWindow { background-color: #2c2f33; }
            QLabel { color: #d4d7dc; font-family: 'Arial'; font-size: 14px; }
            QPushButton {
                background-color: #40444b; color: #d4d7dc; border: none;
                padding: 8px 16px; border-radius: 6px; font-family: 'Arial';
                font-size: 13px; transition: background-color 0.2s;
            }
            QPushButton:hover { background-color: #5865f2; }
            QPushButton:pressed { background-color: #4752c4; }
            QCheckBox { color: #d4d7dc; padding: 4px; font-family: 'Arial'; font-size: 13px; }
            QGroupBox { 
                color: #d4d7dc; font-family: 'Arial'; font-size: 14px; 
                border: 1px solid #40444b; border-radius: 6px; padding: 10px; 
                background-color: #36393f; 
            }
            QSlider::groove:horizontal { background: #40444b; height: 6px; border-radius: 3px; }
            QSlider::handle:horizontal { background: #5865f2; width: 14px; height: 14px; border-radius: 7px; }
            QLineEdit { 
                background-color: #36393f; color: #d4d7dc; border: 1px solid #40444b; 
                border-radius: 6px; padding: 4px; font-family: 'Arial'; font-size: 13px; 
            }
            QToolTip { 
                background-color: #2f3136; color: #d4d7dc; border: 1px solid #40444b; 
                border-radius: 4px; font-family: 'Arial'; font-size: 12px; padding: 4px; 
            }
            QProgressBar {
                background-color: #36393f; border: 1px solid #40444b; border-radius: 6px;
                text-align: center; color: #d4d7dc; font-family: 'Arial'; font-size: 12px;
            }
            QProgressBar::chunk { background-color: #5865f2; border-radius: 4px; }
        """)

        # Параметры изображения
        self.rotation_angle = 0
        self.flip_horizontal = False
        self.flip_vertical = False

        # Параметры сеанса
        self.session_running = False
        self.paused = False
        self.session_order = []
        self.session_total = 0
        self.session_index = 0
        self.countdown = 0
        self.interval_value = 30
        self.image_count_value = 10
        self.in_break = False
        self.break_countdown = 0
        self.shown_history = set()
        self.image_list = []
        self.current_index = 0
        self.pixmap_cache = {}
        self.transformed_cache = {}
        self.loader_thread = None

        # Таймеры
        self.countdown_timer = QTimer(self)
        self.countdown_timer.timeout.connect(self.update_countdown)
        self.break_timer = QTimer(self)
        self.break_timer.timeout.connect(self.update_break)
        self.hide_timer = QTimer(self)
        self.hide_timer.setSingleShot(True)
        self.hide_timer.timeout.connect(self.hide_interface)

        # Основной интерфейс
        self.setup_ui()

    def setup_ui(self):
        # Центральный виджет
        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)

        # Верхняя панель
        self.top_panel = QWidget(self)
        self.top_panel.setStyleSheet("background-color: #36393f; border-bottom: 1px solid #40444b;")
        self.top_panel.setFixedHeight(60)
        top_layout = QHBoxLayout(self.top_panel)
        top_layout.setContentsMargins(15, 0, 15, 0)
        top_layout.setSpacing(10)

        self.status_label = QLabel("Сеанс не запущен")
        self.status_label.setStyleSheet("font-size: 16px; font-weight: bold;")
        top_layout.addWidget(self.status_label)

        self.progress_bar = QProgressBar(self)
        self.progress_bar.setMaximum(100)
        self.progress_bar.setValue(0)
        self.progress_bar.setFixedWidth(200)
        self.progress_bar.setVisible(False)
        top_layout.addWidget(self.progress_bar)

        top_layout.addStretch()

        self.settings_btn = QPushButton("⚙ Настройки")
        self.settings_btn.clicked.connect(self.toggle_settings)
        self.settings_btn.setToolTip("Открыть/закрыть настройки")
        top_layout.addWidget(self.settings_btn)

        self.always_on_top_btn = QPushButton("📌 Всегда сверху")
        self.always_on_top_btn.clicked.connect(self.toggle_always_on_top)
        self.always_on_top_btn.setToolTip("Закрепить окно поверх других")
        top_layout.addWidget(self.always_on_top_btn)

        main_layout.addWidget(self.top_panel)

        # Контейнер изображения
        self.image_container = QWidget(self)
        self.image_container.setStyleSheet("background-color: #21252b;")
        image_layout = QVBoxLayout(self.image_container)
        image_layout.setContentsMargins(15, 15, 15, 15)

        self.image_label = ZoomableLabel(self)
        image_layout.addWidget(self.image_label)
        main_layout.addWidget(self.image_container)

        # Нижняя панель управления
        self.bottom_panel = QWidget(self)
        self.bottom_panel.setStyleSheet("background-color: #36393f; border-top: 1px solid #40444b;")
        bottom_layout = QVBoxLayout(self.bottom_panel)
        bottom_layout.setContentsMargins(15, 10, 15, 10)
        bottom_layout.setSpacing(10)

        # Верхний ряд кнопок (полупрозрачный фон)
        self.top_controls = QWidget(self)
        self.top_controls.setObjectName("topControls")
        self.top_controls.setStyleSheet("""
            .QWidget#topControls { 
                background-color: rgba(64, 68, 75, 180); 
                border-radius: 8px; 
                padding: 5px; 
            }
        """)
        top_controls_layout = QHBoxLayout(self.top_controls)
        top_controls_layout.setContentsMargins(5, 5, 5, 5)
        top_controls_layout.setSpacing(8)

        self.prev_btn = QPushButton("⬅ Предыдущее")
        self.prev_btn.clicked.connect(lambda: self.show_prev_image(manual=True))
        self.prev_btn.setToolTip("Перейти к предыдущему изображению")
        top_controls_layout.addWidget(self.prev_btn)

        self.skip_btn = QPushButton("⏹ Пропустить")
        self.skip_btn.clicked.connect(self.skip_current_image)
        self.skip_btn.setToolTip("Пропустить текущее изображение")
        top_controls_layout.addWidget(self.skip_btn)

        self.pause_btn = QPushButton("⏸ Пауза")
        self.pause_btn.clicked.connect(self.toggle_pause)
        self.pause_btn.setToolTip("Приостановить или возобновить сеанс")
        top_controls_layout.addWidget(self.pause_btn)

        self.next_btn = QPushButton("Следующее ➡")
        self.next_btn.clicked.connect(lambda: self.show_next_image(manual=True))
        self.next_btn.setToolTip("Перейти к следующему изображению")
        top_controls_layout.addWidget(self.next_btn)

        bottom_layout.addWidget(self.top_controls)

        # Нижний ряд кнопок
        bottom_controls_layout = QHBoxLayout()
        bottom_controls_layout.setSpacing(8)

        self.delete_btn = QPushButton("🗑 Удалить")
        self.delete_btn.clicked.connect(self.delete_current_image)
        self.delete_btn.setToolTip("Удалить текущее изображение в корзину")
        bottom_controls_layout.addWidget(self.delete_btn)

        self.rotate_btn = QPushButton("🔄 Повернуть")
        self.rotate_btn.clicked.connect(self.rotate_image)
        self.rotate_btn.setToolTip("Повернуть изображение на 90°")
        bottom_controls_layout.addWidget(self.rotate_btn)

        self.flip_h_btn = QPushButton("↔ Горизонталь")
        self.flip_h_btn.clicked.connect(self.flip_horizontal_action)
        self.flip_h_btn.setToolTip("Отразить изображение по горизонтали")
        bottom_controls_layout.addWidget(self.flip_h_btn)

        self.flip_v_btn = QPushButton("↕ Вертикаль")
        self.flip_v_btn.clicked.connect(self.flip_vertical_action)
        self.flip_v_btn.setToolTip("Отразить изображение по вертикали")
        bottom_controls_layout.addWidget(self.flip_v_btn)

        self.reset_btn = QPushButton("🔧 Сброс")
        self.reset_btn.clicked.connect(self.reset_transformations)
        self.reset_btn.setToolTip("Сбросить все трансформации")
        bottom_controls_layout.addWidget(self.reset_btn)

        self.zoom_reset_btn = QPushButton("🔍 Сброс зума")
        self.zoom_reset_btn.clicked.connect(self.image_label.resetZoom)
        self.zoom_reset_btn.setToolTip("Сбросить уровень зума")
        bottom_controls_layout.addWidget(self.zoom_reset_btn)

        bottom_layout.addLayout(bottom_controls_layout)
        main_layout.addWidget(self.bottom_panel)

        # Панель настроек
        self.settings_panel = QWidget(self)
        self.settings_panel.setStyleSheet("""
            background-color: #36393f; 
            border: 1px solid #40444b; 
            border-radius: 10px; 
            padding: 10px;
        """)
        self.settings_panel.setFixedSize(400, 500)
        self.settings_panel.setVisible(False)
        settings_layout = QVBoxLayout(self.settings_panel)
        settings_layout.setContentsMargins(20, 20, 20, 20)
        settings_layout.setSpacing(15)

        folder_group = QGroupBox("Выбор папки")
        folder_layout = QVBoxLayout()
        self.folder_btn = QPushButton("📂 Выбрать папку")
        self.folder_btn.clicked.connect(self.choose_folder)
        self.folder_btn.setStyleSheet("background-color: #5865f2;")
        folder_layout.addWidget(self.folder_btn)
        folder_group.setLayout(folder_layout)
        settings_layout.addWidget(folder_group)

        interval_group = QGroupBox("Интервал (сек)")
        interval_layout = QHBoxLayout()
        self.interval_slider = QSlider(Qt.Orientation.Horizontal)
        self.interval_slider.setRange(1, 300)
        self.interval_slider.setValue(self.interval_value)
        self.interval_slider.valueChanged.connect(self.update_interval)
        self.interval_edit = QLineEdit(str(self.interval_value))
        self.interval_edit.setFixedWidth(60)
        self.interval_edit.editingFinished.connect(self.update_interval_from_edit)
        interval_layout.addWidget(self.interval_slider)
        interval_layout.addWidget(self.interval_edit)
        interval_group.setLayout(interval_layout)
        settings_layout.addWidget(interval_group)

        self.unlimited_interval_cb = QCheckBox("Ручной переход")
        self.unlimited_interval_cb.setToolTip("Переключать изображения вручную")
        settings_layout.addWidget(self.unlimited_interval_cb)

        count_group = QGroupBox("Количество изображений")
        count_layout = QHBoxLayout()
        self.count_slider = QSlider(Qt.Orientation.Horizontal)
        self.count_slider.setRange(5, 40)
        self.count_slider.setValue(self.image_count_value)
        self.count_slider.valueChanged.connect(self.update_count)
        self.count_edit = QLineEdit(str(self.image_count_value))
        self.count_edit.setFixedWidth(60)
        self.count_edit.editingFinished.connect(self.update_count_from_edit)
        count_layout.addWidget(self.count_slider)
        count_layout.addWidget(self.count_edit)
        count_group.setLayout(count_layout)
        settings_layout.addWidget(count_group)

        self.unlimited_count_cb = QCheckBox("Неограниченное количество")
        self.unlimited_count_cb.setToolTip("Использовать все изображения в папке")
        settings_layout.addWidget(self.unlimited_count_cb)

        self.no_repeat_cb = QCheckBox("Без повторов")
        self.no_repeat_cb.setToolTip("Не повторять изображения в сеансе")
        settings_layout.addWidget(self.no_repeat_cb)

        self.breaks_cb = QCheckBox("Перерывы (5 мин)")
        self.breaks_cb.setToolTip("Делать перерывы каждые 30 минут")
        settings_layout.addWidget(self.breaks_cb)

        self.start_stop_btn = QPushButton("▶ Запустить")
        self.start_stop_btn.clicked.connect(self.toggle_session)
        self.start_stop_btn.setStyleSheet("background-color: #5865f2; font-weight: bold;")
        self.start_stop_btn.setToolTip("Запустить или остановить сеанс")
        settings_layout.addWidget(self.start_stop_btn)
        settings_layout.addStretch()

        # Центральная кнопка действия
        self.action_btn = QPushButton("", self.image_container)
        self.action_btn.setVisible(False)
        self.action_btn.clicked.connect(self.handle_action)
        self.action_btn.setStyleSheet("""
            background-color: #5865f2; 
            font-size: 16px; 
            font-weight: bold; 
            border-radius: 8px;
        """)
        self.action_btn.setFixedSize(220, 50)

        # Горячие клавиши
        shortcuts = {
            "Left": lambda: self.show_prev_image(manual=True),
            "Right": lambda: self.show_next_image(manual=True),
            "Delete": self.skip_current_image,
            "Backspace": self.delete_current_image,
            "R": self.rotate_image,
            "H": self.flip_horizontal_action,
            "V": self.flip_vertical_action,
            "T": self.reset_transformations,
            "Z": self.image_label.resetZoom
        }
        for key, func in shortcuts.items():
            QShortcut(QKeySequence(key), self, activated=func)

    def toggle_settings(self):
        visible = not self.settings_panel.isVisible()
        self.settings_panel.setVisible(visible)
        if visible:
            self.center_widget(self.settings_panel)
            anim = QPropertyAnimation(self.settings_panel, b"windowOpacity")
            anim.setDuration(300)
            anim.setStartValue(0)
            anim.setEndValue(1)
            anim.start()

    def center_widget(self, widget):
        x = (self.image_container.width() - widget.width()) // 2
        y = (self.image_container.height() - widget.height()) // 2
        widget.move(x, y)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.center_widget(self.settings_panel)
        self.center_widget(self.action_btn)

    def disable_controls(self):
        for btn in [self.prev_btn, self.next_btn, self.skip_btn, self.delete_btn,
                    self.rotate_btn, self.flip_h_btn, self.flip_v_btn, self.reset_btn, self.zoom_reset_btn]:
            btn.setEnabled(False)

    def enable_controls(self):
        for btn in [self.prev_btn, self.next_btn, self.skip_btn, self.delete_btn,
                    self.rotate_btn, self.flip_h_btn, self.flip_v_btn, self.reset_btn, self.zoom_reset_btn]:
            btn.setEnabled(True)

    def update_interval(self, value):
        self.interval_value = value
        self.interval_edit.setText(str(value))

    def update_interval_from_edit(self):
        try:
            value = int(self.interval_edit.text())
            value = max(1, min(300, value))
        except ValueError:
            value = self.interval_value
        self.interval_value = value
        self.interval_slider.setValue(value)
        self.interval_edit.setText(str(value))

    def update_count(self, value):
        self.image_count_value = value
        self.count_edit.setText(str(value))

    def update_count_from_edit(self):
        try:
            value = int(self.count_edit.text())
            value = max(5, min(40, value))
        except ValueError:
            value = self.image_count_value
        self.image_count_value = value
        self.count_slider.setValue(value)
        self.count_edit.setText(str(value))

    def choose_folder(self):
        folder = QFileDialog.getExistingDirectory(self, "Выберите папку", "")
        if folder:
            self.folder_path = folder
            self.scan_folder(folder)

    def scan_folder(self, folder):
        self.stop_session()
        if self.loader_thread and self.loader_thread.isRunning():
            self.loader_thread.quit()
            self.loader_thread.wait()
        self.image_label.setText("Загрузка изображений...")
        worker = ImageLoaderWorker()
        self.loader_thread = QThread()
        worker.moveToThread(self.loader_thread)
        self.loader_thread.started.connect(partial(worker.load_folder, folder))
        worker.finished.connect(self.on_folder_scanned)
        worker.finished.connect(self.loader_thread.quit)
        worker.finished.connect(worker.deleteLater)
        self.loader_thread.finished.connect(self.loader_thread.deleteLater)
        self.loader_thread.start()

    def on_folder_scanned(self, file_list):
        self.image_list = file_list
        if not file_list:
            self.image_label.setText("Изображений не найдено")
        else:
            self.pixmap_cache.clear()
            self.transformed_cache.clear()
            self.current_index = 0
            self.display_image(self.image_list[0])

    def toggle_session(self):
        if self.session_running:
            self.stop_session()
        else:
            self.start_session()

    def start_session(self):
        if not self.image_list:
            self.status_label.setText("Ошибка: выберите папку!")
            QToolTip.showText(self.mapToGlobal(self.status_label.pos()), "Выберите папку с изображениями", self, QRectF(), 2000)
            return
        available = list(set(range(len(self.image_list))) - self.shown_history if self.no_repeat_cb.isChecked() else range(len(self.image_list)))
        if not available:
            self.shown_history.clear()
            available = list(range(len(self.image_list)))
            self.status_label.setText("История изображений сброшена")
        count = len(available) if self.unlimited_count_cb.isChecked() else min(self.image_count_value, len(available))
        random.shuffle(available)
        self.session_order = available[:count]
        self.session_total = len(self.session_order)
        self.session_index = 0
        self.current_index = self.session_order[0]
        self.countdown = self.interval_value if not self.unlimited_interval_cb.isChecked() else None
        if self.countdown:
            self.countdown_timer.start(1000)
        self.session_running = True
        self.paused = False
        self.start_stop_btn.setText("⏹ Остановить")
        self.status_label.setText(f"Сеанс: {self.session_index + 1}/{self.session_total}")
        self.progress_bar.setVisible(True)
        self.progress_bar.setMaximum(self.session_total)
        self.progress_bar.setValue(self.session_index + 1)
        self.enable_controls()
        self.settings_panel.setVisible(False)
        self.action_btn.setVisible(False)
        self.display_image(self.image_list[self.current_index], animate=True)

    def stop_session(self):
        self.countdown_timer.stop()
        self.session_running = False
        self.paused = False
        self.start_stop_btn.setText("▶ Запустить")
        self.status_label.setText("Сеанс остановлен")
        self.progress_bar.setVisible(False)
        self.disable_controls()

    def update_countdown(self):
        if not self.session_running or self.paused or self.countdown is None:
            return
        self.countdown -= 1
        self.status_label.setText(f"Сеанс: {self.session_index + 1}/{self.session_total} | {self.countdown} сек")
        if self.countdown <= 0:
            self.show_next_image(manual=False)
            self.countdown = self.interval_value

    def show_next_image(self, manual=False):
        if not self.session_running or not self.session_order:
            return
        if self.session_index < self.session_total - 1:
            self.session_index += 1
            self.current_index = self.session_order[self.session_index]
            self.display_image(self.image_list[self.current_index], animate=True)
            self.progress_bar.setValue(self.session_index + 1)
            if manual and self.countdown is not None:
                self.countdown = self.interval_value
                self.status_label.setText(f"Сеанс: {self.session_index + 1}/{self.session_total} | {self.countdown} сек")
        else:
            self.finish_session()

    def show_prev_image(self, manual=False):
        if not self.session_running or not self.session_order:
            return
        if self.session_index > 0:
            self.session_index -= 1
            self.current_index = self.session_order[self.session_index]
            self.display_image(self.image_list[self.current_index], animate=True)
            self.progress_bar.setValue(self.session_index + 1)
            if manual and self.countdown is not None:
                self.countdown = self.interval_value
                self.status_label.setText(f"Сеанс: {self.session_index + 1}/{self.session_total} | {self.countdown} сек")

    def display_image(self, image_path, animate=False):
        if not os.path.exists(image_path):
            self.image_label.setText("Изображение не найдено")
            return
        if image_path in self.pixmap_cache:
            pixmap = self.pixmap_cache[image_path]
        else:
            pixmap = QPixmap(image_path)
            if pixmap.isNull():
                self.image_label.setText("Ошибка загрузки изображения")
                return
            self.pixmap_cache[image_path] = pixmap
        key = (image_path, self.rotation_angle, self.flip_horizontal, self.flip_vertical)
        if key in self.transformed_cache:
            transformed = self.transformed_cache[key]
        else:
            transform = QTransform()
            if self.flip_horizontal:
                transform.scale(-1, 1)
            if self.flip_vertical:
                transform.scale(1, -1)
            transform.rotate(self.rotation_angle)
            transformed = pixmap.transformed(transform, Qt.TransformationMode.SmoothTransformation)
            self.transformed_cache[key] = transformed
        self.image_label.setBasePixmap(transformed)
        if animate:
            anim = QPropertyAnimation(self.image_label, b"windowOpacity")
            anim.setDuration(200)
            anim.setStartValue(0)
            anim.setEndValue(1)
            anim.start()
        if self.session_running and self.no_repeat_cb.isChecked():
            self.shown_history.add(self.current_index)

    def delete_current_image(self):
        if not self.image_list:
            return
        path = self.image_list[self.current_index]
        try:
            send2trash(path)
            self.image_list.pop(self.current_index)
            self.pixmap_cache.pop(path, None)
            self.transformed_cache.clear()
            if self.image_list:
                self.current_index = min(self.current_index, len(self.image_list) - 1)
                self.display_image(self.image_list[self.current_index], animate=True)
                if self.session_running:
                    self.session_order.pop(self.session_index)
                    self.session_total -= 1
                    self.session_index = min(self.session_index, self.session_total - 1)
                    self.progress_bar.setMaximum(self.session_total)
                    self.progress_bar.setValue(self.session_index + 1)
                    self.status_label.setText(f"Сеанс: {self.session_index + 1}/{self.session_total}")
            else:
                self.image_label.setText("Нет изображений")
                self.stop_session()
        except Exception as e:
            self.status_label.setText(f"Ошибка: {e}")
            QToolTip.showText(self.mapToGlobal(self.status_label.pos()), f"Не удалось удалить: {e}", self, QRectF(), 2000)

    def rotate_image(self):
        self.rotation_angle = (self.rotation_angle + 90) % 360
        self.transformed_cache.clear()
        self.display_image(self.image_list[self.current_index], animate=True)

    def flip_horizontal_action(self):
        self.flip_horizontal = not self.flip_horizontal
        self.transformed_cache.clear()
        self.display_image(self.image_list[self.current_index], animate=True)

    def flip_vertical_action(self):
        self.flip_vertical = not self.flip_vertical
        self.transformed_cache.clear()
        self.display_image(self.image_list[self.current_index], animate=True)

    def reset_transformations(self):
        self.rotation_angle = 0
        self.flip_horizontal = False
        self.flip_vertical = False
        self.transformed_cache.clear()
        self.display_image(self.image_list[self.current_index], animate=True)

    def skip_current_image(self):
        if not self.session_running or not self.session_order:
            return
        self.session_order.pop(self.session_index)
        self.session_total -= 1
        self.progress_bar.setMaximum(self.session_total)
        if not self.session_order:
            self.finish_session()
            return
        self.session_index = min(self.session_index, self.session_total - 1)
        self.current_index = self.session_order[self.session_index]
        self.display_image(self.image_list[self.current_index], animate=True)
        self.progress_bar.setValue(self.session_index + 1)
        self.status_label.setText(f"Сеанс: {self.session_index + 1}/{self.session_total}")

    def finish_session(self):
        self.countdown_timer.stop()
        self.session_running = False
        self.start_stop_btn.setText("▶ Запустить")
        self.progress_bar.setVisible(False)
        if self.breaks_cb.isChecked():
            self.start_break()
        else:
            self.status_label.setText("Сеанс завершён")
            QToolTip.showText(self.mapToGlobal(self.status_label.pos()), "Сеанс успешно завершён!", self, QRectF(), 2000)
            self.action_btn.setText("Новый сеанс")
            self.action_btn.setVisible(True)
            self.center_widget(self.action_btn)
        self.disable_controls()

    def start_break(self):
        self.in_break = True
        self.break_countdown = 300
        self.break_timer.start(1000)
        self.status_label.setText(f"Перерыв: {self.break_countdown // 60:02d}:{self.break_countdown % 60:02d}")
        self.action_btn.setText("Пропустить перерыв")
        self.action_btn.setVisible(True)
        self.center_widget(self.action_btn)

    def update_break(self):
        self.break_countdown -= 1
        self.status_label.setText(f"Перерыв: {self.break_countdown // 60:02d}:{self.break_countdown % 60:02d}")
        if self.break_countdown <= 0:
            self.break_timer.stop()
            self.in_break = False
            self.status_label.setText("Перерыв завершён")
            self.action_btn.setText("Новый сеанс")
            QToolTip.showText(self.mapToGlobal(self.status_label.pos()), "Перерыв окончен, начните новый сеанс!", self, QRectF(), 2000)

    def handle_action(self):
        self.action_btn.setVisible(False)
        if self.in_break:
            self.break_timer.stop()
            self.in_break = False
            self.status_label.setText("Перерыв пропущен")
            self.action_btn.setText("Новый сеанс")
        else:
            self.start_session()

    def toggle_always_on_top(self):
        flag = Qt.WindowType.WindowStaysOnTopHint
        if self.windowFlags() & flag:
            self.setWindowFlag(flag, False)
            self.always_on_top_btn.setText("📌 Всегда сверху")
        else:
            self.setWindowFlag(flag, True)
            self.always_on_top_btn.setText("📍 Обычный")
        self.show()

    def leaveEvent(self, event):
        self.hide_timer.start(1000)

    def enterEvent(self, event):
        self.hide_timer.stop()
        self.show_interface()

    def hide_interface(self):
        self.bottom_panel.setVisible(False)
        self.settings_panel.setVisible(False)
        if not self.session_running:
            self.action_btn.setVisible(True)
        else:
            self.action_btn.setVisible(False)

    def show_interface(self):
        self.bottom_panel.setVisible(True)
        if self.settings_panel.isVisible():
            self.settings_panel.setVisible(True)
        if not self.session_running:
            self.action_btn.setVisible(True)
        else:
            self.action_btn.setVisible(False)

    def toggle_pause(self):
        if not self.session_running:
            return
        self.paused = not self.paused
        self.pause_btn.setText("▶ Возобновить" if self.paused else "⏸ Пауза")
        if self.paused:
            self.countdown_timer.stop()
            self.status_label.setText(f"Сеанс на паузе: {self.session_index + 1}/{self.session_total}")
        else:
            self.countdown_timer.start(1000)
            self.status_label.setText(f"Сеанс: {self.session_index + 1}/{self.session_total} | {self.countdown} сек")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    QToolTip.setFont(QFont("Arial", 12))
    window = GesturePosesApp()
    window.show()
    sys.exit(app.exec())
