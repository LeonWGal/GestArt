"""Emerald theme colors and styles"""

def get_colors():
    return {
        # Основные цвета
        "background": "rgba(230, 245, 230, 250)",
        "background_secondary": "rgba(200, 230, 200, 180)",
        "background_hover": "rgba(180, 220, 180, 200)",
        "background_pressed": "rgba(160, 200, 160, 250)",
        "background_checked": "rgba(60, 150, 90, 200)",
        
        # Цвета текста
        "text": "rgba(20, 80, 20, 250)",
        "text_secondary": "rgba(40, 100, 40, 250)",
        "text_disabled": "rgba(100, 140, 100, 250)",
        
        # Цвета границ
        "border": "rgba(140, 200, 140, 250)",
        "border_hover": "rgba(100, 180, 100, 250)",
        "border_focus": "rgba(60, 150, 90, 250)",
        
        # Цвета акцента
        "accent": "rgba(60, 150, 90, 200)",
        "accent_hover": "rgba(80, 170, 110, 200)",
        "accent_pressed": "rgba(50, 130, 80, 200)",
        
        # Цвета оверлея
        "overlay_background": "rgba(230, 250, 230, 200)",
        "overlay_text": "rgba(20, 80, 20, 250)",
        
        # Цвета для специальных элементов
        "timer_background": "rgba(220, 240, 220, 200)",
        "grid_color": "rgba(40, 120, 40, 250)",
        "separator": "rgba(140, 200, 140, 250)",
        
        # Цвета для подсказок (высокий контраст)
        "tooltip_background": "rgba(35, 45, 35, 250)",
        "tooltip_text": "rgba(255, 255, 255, 255)",
        "tooltip_border": "rgba(100, 200, 100, 200)"
    } 