"""Sapphire theme colors and styles"""

def get_colors():
    return {
        "background": "rgba(15, 20, 35, 250)",  # Темный фон с синим оттенком
        "background_secondary": "rgba(25, 30, 45, 220)",  # Вторичный фон
        "background_hover": "rgba(35, 40, 55, 240)",  # При наведении
        "background_pressed": "rgba(45, 50, 65, 250)",  # При нажатии
        "background_checked": "rgba(0, 100, 200, 160)",  # Активный элемент (синий)
        
        "text": "rgba(255, 255, 255, 240)",  # Белый текст
        "text_secondary": "rgba(200, 200, 200, 200)",  # Вторичный текст
        "text_disabled": "rgba(120, 120, 120, 140)",  # Отключенный текст
        
        "border": "rgba(0, 100, 200, 150)",  # Синяя граница
        "border_hover": "rgba(0, 100, 200, 200)",  # При наведении
        "border_focus": "rgba(50, 150, 255, 220)",  # При фокусе (светло-синий)
        
        "accent": "rgba(0, 100, 200, 200)",  # Основной акцент (синий)
        "accent_hover": "rgba(0, 100, 200, 240)",  # При наведении
        "accent_pressed": "rgba(0, 80, 160, 250)",  # При нажатии
        
        "accent_blue": "rgba(0, 100, 200, 200)",  # Синий акцент
        "accent_yellow": "rgba(255, 200, 50, 200)",  # Желтый акцент
        
        "overlay_background": "rgba(0, 0, 0, 180)",  # Полупрозрачный черный
        "overlay_text": "rgba(255, 255, 255, 240)",  # Белый текст на оверлее
        
        "timer_background": "rgba(0, 100, 200, 180)",  # Фон таймера (синий)
        "grid_color": "rgba(0, 100, 200, 150)",  # Цвет сетки (синий)
        "separator": "rgba(50, 150, 255, 150)",  # Разделители (светло-синий)
        
        "shadow": "rgba(0, 0, 0, 60)",  # Тень
        "highlight": "rgba(255, 255, 255, 80)",  # Подсветка
        "success": "rgba(76, 175, 80, 200)",  # Успех
        "warning": "rgba(255, 193, 7, 200)",  # Предупреждение
        "error": "rgba(244, 67, 54, 200)",  # Ошибка
        
        # Цвета для подсказок (высокий контраст)
        "tooltip_background": "rgba(25, 35, 50, 250)",
        "tooltip_text": "rgba(255, 255, 255, 255)",
        "tooltip_border": "rgba(50, 150, 255, 200)"
    }
