"""Jasper theme colors and styles"""

def get_colors():
    return {
        "background": "rgba(25, 20, 15, 250)",  # Темный фон с оранжевым оттенком
        "background_secondary": "rgba(35, 30, 25, 220)",  # Вторичный фон
        "background_hover": "rgba(45, 40, 35, 240)",  # При наведении
        "background_pressed": "rgba(55, 50, 45, 250)",  # При нажатии
        "background_checked": "rgba(255, 140, 0, 160)",  # Активный элемент (оранжевый)
        
        "text": "rgba(255, 255, 255, 240)",  # Белый текст
        "text_secondary": "rgba(200, 200, 200, 200)",  # Вторичный текст
        "text_disabled": "rgba(120, 120, 120, 140)",  # Отключенный текст
        
        "border": "rgba(255, 140, 0, 150)",  # Оранжевая граница
        "border_hover": "rgba(255, 140, 0, 200)",  # При наведении
        "border_focus": "rgba(255, 180, 50, 220)",  # При фокусе (светло-оранжевый)
        
        "accent": "rgba(255, 140, 0, 200)",  # Основной акцент (оранжевый)
        "accent_hover": "rgba(255, 140, 0, 240)",  # При наведении
        "accent_pressed": "rgba(220, 120, 0, 250)",  # При нажатии
        
        "accent_blue": "rgba(100, 150, 255, 200)",  # Синий акцент
        "accent_yellow": "rgba(255, 200, 50, 200)",  # Желтый акцент
        
        "overlay_background": "rgba(0, 0, 0, 180)",  # Полупрозрачный черный
        "overlay_text": "rgba(255, 255, 255, 240)",  # Белый текст на оверлее
        
        "timer_background": "rgba(255, 140, 0, 180)",  # Фон таймера (оранжевый)
        "grid_color": "rgba(255, 140, 0, 150)",  # Цвет сетки (оранжевый)
        "separator": "rgba(255, 180, 50, 150)",  # Разделители (светло-оранжевый)
        
        "shadow": "rgba(0, 0, 0, 60)",  # Тень
        "highlight": "rgba(255, 255, 255, 80)",  # Подсветка
        "success": "rgba(76, 175, 80, 200)",  # Успех
        "warning": "rgba(255, 193, 7, 200)",  # Предупреждение
        "error": "rgba(244, 67, 54, 200)",  # Ошибка
        
        # Цвета для подсказок (высокий контраст)
        "tooltip_background": "rgba(50, 35, 25, 250)",
        "tooltip_text": "rgba(255, 255, 255, 255)",
        "tooltip_border": "rgba(255, 180, 50, 200)"
    }
