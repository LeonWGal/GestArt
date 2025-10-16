"""Dark theme colors and styles"""

def get_colors():
    return {
        # Основные цвета
        "background": "rgba(30, 30, 30, 250)",
        "background_secondary": "rgba(45, 45, 45, 180)",
        "background_hover": "rgba(55, 55, 55, 200)",
        "background_pressed": "rgba(35, 35, 35, 200)",
        "background_checked": "rgba(70, 130, 180, 200)",
        
        # Цвета текста
        "text": "#ffffff",
        "text_secondary": "rgba(255, 255, 255, 180)",
        "text_disabled": "rgba(255, 255, 255, 120)",
        
        # Цвета границ
        "border": "rgba(255, 255, 255, 20)",
        "border_hover": "rgba(255, 255, 255, 30)",
        "border_focus": "rgba(255, 255, 255, 50)",
        
        # Цвета акцента
        "accent": "rgba(70, 130, 180, 200)",
        "accent_hover": "rgba(90, 150, 200, 200)",
        "accent_pressed": "rgba(60, 120, 170, 200)",
        
        # Цвета оверлея
        "overlay_background": "rgba(0, 0, 0, 120)",
        "overlay_text": "#ffffff",
        
        # Цвета для специальных элементов
        "timer_background": "rgba(0, 0, 0, 120)",
        "grid_color": "#ffffff",
        "separator": "rgba(255, 255, 255, 30)",
        
        # Цвета для подсказок (высокий контраст)
        "tooltip_background": "rgba(45, 45, 45, 250)",
        "tooltip_text": "rgba(255, 255, 255, 255)",
        "tooltip_border": "rgba(255, 255, 255, 60)"
    } 