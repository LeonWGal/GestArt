"""Violet theme colors and styles"""

def get_colors():
    return {
        # Основные цвета
        "background": "rgba(40, 30, 60, 250)",
        "background_secondary": "rgba(55, 40, 80, 180)",
        "background_hover": "rgba(65, 50, 90, 200)",
        "background_pressed": "rgba(45, 35, 70, 200)",
        "background_checked": "rgba(120, 80, 180, 200)",
        
        # Цвета текста
        "text": "rgba(235, 225, 255, 250)",
        "text_secondary": "rgba(200, 190, 230, 250)",
        "text_disabled": "rgba(150, 140, 180, 250)",
        
        # Цвета границ
        "border": "rgba(100, 80, 150, 250)",
        "border_hover": "rgba(130, 100, 200, 250)",
        "border_focus": "rgba(150, 120, 220, 250)",
        
        # Цвета акцента
        "accent": "rgba(130, 90, 210, 200)",
        "accent_hover": "rgba(150, 110, 230, 200)",
        "accent_pressed": "rgba(110, 70, 190, 200)",
        
        # Цвета оверлея
        "overlay_background": "rgba(30, 20, 50, 200)",
        "overlay_text": "rgba(235, 225, 255, 250)",
        
        # Цвета для специальных элементов
        "timer_background": "rgba(40, 30, 60, 200)",
        "grid_color": "rgba(180, 160, 240, 250)",
        "separator": "rgba(100, 80, 150, 250)",
        
        # Цвета для подсказок (высокий контраст)
        "tooltip_background": "rgba(45, 35, 55, 250)",
        "tooltip_text": "rgba(255, 255, 255, 255)",
        "tooltip_border": "rgba(180, 160, 240, 200)"
    } 