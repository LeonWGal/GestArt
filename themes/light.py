"""Light theme colors and styles"""

def get_colors():
    return {
        # Основные цвета
        "background": "rgba(250, 250, 250, 250)",
        "background_secondary": "rgba(240, 240, 240, 180)",
        "background_hover": "rgba(230, 230, 230, 200)",
        "background_pressed": "rgba(220, 220, 220, 250)",
        "background_checked": "rgba(70, 130, 180, 250)",
        
        # Цвета текста
        "text": "rgba(30, 30, 30, 250)",
        "text_secondary": "rgba(60, 60, 60, 250)",
        "text_disabled": "rgba(120, 120, 120, 250)",
        
        # Цвета границ
        "border": "rgba(200, 200, 200, 250)",
        "border_hover": "rgba(180, 180, 180, 250)",
        "border_focus": "rgba(70, 130, 180, 250)",
        
        # Цвета акцента
        "accent": "rgba(70, 130, 180, 250)",
        "accent_hover": "rgba(90, 150, 200, 250)",
        "accent_pressed": "rgba(50, 110, 160, 250)",
        
        # Цвета оверлея
        "overlay_background": "rgba(255, 255, 255, 200)",
        "overlay_text": "rgba(30, 30, 30, 250)",
        
        # Цвета для специальных элементов
        "timer_background": "rgba(255, 255, 255, 200)",
        "grid_color": "rgba(30, 30, 30, 250)",
        "separator": "rgba(200, 200, 200, 250)",
        
        # Цвета для подсказок (высокий контраст)
        "tooltip_background": "rgba(35, 35, 35, 250)",
        "tooltip_text": "rgba(255, 255, 255, 255)",
        "tooltip_border": "rgba(70, 130, 180, 180)"
    } 