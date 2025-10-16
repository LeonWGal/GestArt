"""Ruby Light theme colors and styles"""

def get_colors():
    return {
        "background": "rgba(255, 250, 250, 250)",  # Светлый фон с легким красным оттенком
        "background_secondary": "rgba(245, 240, 240, 220)",  # Вторичный фон
        "background_hover": "rgba(235, 230, 230, 240)",  # При наведении
        "background_pressed": "rgba(225, 220, 220, 250)",  # При нажатии
        "background_checked": "rgba(220, 20, 60, 160)",  # Активный элемент (красный)
        
        "text": "rgba(30, 30, 40, 240)",  # Темный текст
        "text_secondary": "rgba(60, 60, 70, 200)",  # Вторичный текст
        "text_disabled": "rgba(120, 120, 130, 140)",  # Отключенный текст
        
        "border": "rgba(220, 20, 60, 150)",  # Красная граница
        "border_hover": "rgba(220, 20, 60, 200)",  # При наведении
        "border_focus": "rgba(255, 100, 100, 220)",  # При фокусе (светло-красный)
        
        "accent": "rgba(220, 20, 60, 200)",  # Основной акцент (красный)
        "accent_hover": "rgba(220, 20, 60, 240)",  # При наведении
        "accent_pressed": "rgba(180, 10, 40, 250)",  # При нажатии
        
        "accent_blue": "rgba(100, 150, 255, 200)",  # Синий акцент
        "accent_yellow": "rgba(255, 200, 50, 200)",  # Желтый акцент
        
        "overlay_background": "rgba(255, 255, 255, 180)",  # Полупрозрачный белый
        "overlay_text": "rgba(30, 30, 40, 240)",  # Темный текст на оверлее
        
        "timer_background": "rgba(220, 20, 60, 180)",  # Фон таймера (красный)
        "grid_color": "rgba(220, 20, 60, 150)",  # Цвет сетки (красный)
        "separator": "rgba(255, 100, 100, 150)",  # Разделители (светло-красный)
        
        "shadow": "rgba(0, 0, 0, 60)",  # Тень
        "highlight": "rgba(255, 255, 255, 80)",  # Подсветка
        "success": "rgba(76, 175, 80, 200)",  # Успех
        "warning": "rgba(255, 193, 7, 200)",  # Предупреждение
        "error": "rgba(244, 67, 54, 200)",  # Ошибка
        
        # Цвета для подсказок (высокий контраст)
        "tooltip_background": "rgba(45, 25, 25, 250)",
        "tooltip_text": "rgba(255, 255, 255, 255)",
        "tooltip_border": "rgba(255, 100, 100, 200)"
    }
