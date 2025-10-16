import json
import os
from typing import Dict, Optional
import logging

class TranslationManager:
    def __init__(self):
        self.current_language: str = "en"
        self.translations: Dict[str, Dict[str, str]] = {}
        self.available_languages: Dict[str, str] = {
            # Основные языки (в начале)
            "en": "English",
            "ru": "Русский",
            
            # Остальные языки по алфавиту
            "af": "Afrikaans",
            "am": "አማርኛ",
            "ar": "العربية",
            "av": "Авар мацӏ",
            "az": "Azərbaycan",
            "ba": "Башҡортса",
            "be": "Беларуская",
            "bg": "Български",
            "bn": "বাংলা",
            "bua": "Буряад хэлэн",
            "ca": "Català",
            "ce": "Нохчийн мотт",
            "chv": "Чӑваш чӗлхи",
            "cs": "Čeština",
            "da": "Dansk",
            
            "de": "Deutsch",
            "el": "Ελληνικά",
            
            "es": "Español",
            "et": "Eesti",
            "fa": "فارسی",
            "ff": "Fulfulde",
            "fi": "Suomi",
            "fr": "Français",
            "gu": "ગુજરાતી",
            "ha": "Hausa",
            "he": "עברית",
            "hi": "हिन्दी",
            "hr": "Hrvatski",
            "ht": "Kreyòl ayisyen",
            "hu": "Magyar",
            "hy": "Հայերեն",
            "id": "Bahasa Indonesia",
            "ig": "Igbo",
            "it": "Italiano",
            "ja": "日本語",
            "jv": "Basa Jawa",
            "ka": "ქართული",
            "kbd": "Адыгэбзэ",
            "kk": "Қазақша",
            "km": "ខ្មែរ",
            "kn": "ಕನ್ನಡ",
            "ko": "한국어",
            "krc": "Къарачай тил",
            "kum": "Къумукъ тил",
            
            "ky": "Кыргызча",
            "lez": "Лезги чӏал",
            "lo": "ລາວ",
            "lt": "Lietuvių",
            "lv": "Latviešu",
            
            "mg": "Malagasy",
            "mhr": "Марий йылме",
            "ml": "മലയാളം",
            "mn": "Монгол",
            "mr": "मराठी",
            "ms": "Bahasa Melayu",
            "my": "မြန်မာ",
            
            "ne": "नेपाली",
            "nl": "Nederlands",
            "no": "Norsk",
            "om": "Afaan Oromoo",
            "os": "Ирон ӕвзаг",
            "pa": "ਪੰਜਾਬੀ",
            "pl": "Polski",
            "pt": "Português",
            "pt-BR": "Português (Brasil)",
            "qu": "Runa Simi",
            "ro": "Română",
            "sah": "Саха тыла",
            "si": "සිංහල",
            "sk": "Slovenčina",
            "sl": "Slovenščina",
            "sq": "Shqip",
            "sr": "Српски",
            "sv": "Svenska",
            "sw": "Kiswahili",
            "is": "Íslenska",
            "ku": "Kurdî",
            "ps": "پښتو",
            "so": "Soomaali",
            "su": "Basa Sunda",
            "ta": "தமிழ்",
            "te": "తెలుగు",
            "th": "ไทย",
            "tk": "Türkmen",
            "tl": "Tagalog",
            "tr": "Türkçe",
            "tt": "Татарча",
            "ug": "Uyghur",
            "tg": "Тоҷикӣ",
            "udm": "Удмурт кыл",
            "uk": "Українська",
            "ur": "اردو",
            "uz": "O'zbekcha",
            "vi": "Tiếng Việt",
            "yo": "Yorùbá",
            "zh-CN": "中文 (简体)",
            "zh-TW": "中文 (繁體)",
            "zu": "IsiZulu"
        }
        self.load_translations()

    def load_translations(self) -> None:
        """Loading available translations"""
        locales_dir = os.path.dirname(os.path.abspath(__file__))
        
        # Loading translations for each language
        for item in os.listdir(locales_dir):
            if item.endswith('.json'):
                locale = item[:-5]  # Removing .json extension
                locale_file = os.path.join(locales_dir, item)
                try:
                    with open(locale_file, "r", encoding="utf-8") as f:
                        self.translations[locale] = json.load(f)
                except FileNotFoundError:
                    logging.warning(f"Translation file not found: {item}")
                except json.JSONDecodeError:
                    logging.error(f"Invalid JSON in translation file: {locale_file}")
                except Exception as e:
                    logging.error(f"Error loading translations for {locale}: {e}")

    def set_locale(self, locale: str) -> None:
        """Setting current language"""
        if locale in self.translations:
            self.current_language = locale
            logging.info(f"Language set to: {locale}")
        else:
            logging.warning(f"Requested language '{locale}' not available, using '{self.current_language}'")

    def load_language(self, locale: str) -> None:
        """Alias for set_locale for better API consistency"""
        self.set_locale(locale)

    def get_locale(self) -> str:
        """Getting current language"""
        return self.current_language

    def get_available_languages(self) -> Dict[str, str]:
        """Getting dictionary of available languages with their display names
        Sorted: en, ru first, then others by display name.
        """
        # Separate priority and others
        priority = {k: self.available_languages[k] for k in ["en", "ru"] if k in self.available_languages}
        rest_items = [(k, v) for k, v in self.available_languages.items() if k not in priority]
        # Sort others by display name (case-insensitive)
        rest_items.sort(key=lambda kv: kv[1].lower())
        ordered = {**priority, **{k: v for k, v in rest_items}}
        return ordered

    def translate(self, text: str) -> str:
        """Returns translation for given text.
        Always tries to fetch from the current locale (including 'en').
        Falls back to original text if key is missing.
        """
        # Try to fetch translation for the current language
        translation = self.translations.get(self.current_language, {}).get(text)
        
        # Fallback to English translation value if missing in current language
        if translation is None:
            translation = self.translations.get("en", {}).get(text)
        
        # If there's no translation but the text contains a line break, check without it
        if translation is None and "\n" in text:
            # Try to find translation for text without line breaks
            flat_text = text.replace("\n", " ")
            translation = self.translations.get(self.current_language, {}).get(flat_text)
            
            # If we found a translation, add line break at the same position
            if translation is not None:
                words = translation.split()
                if len(words) >= 2:
                    # Determine position of line break in original text
                    first_line_length = text.find("\n")
                    if first_line_length > 0:
                        # Add line break after first word
                        translation = words[0] + "\n" + " ".join(words[1:])
        
        return translation if translation is not None else text

    def save_translations(self) -> None:
        """Saving translations to files"""
        locales_dir = os.path.dirname(os.path.abspath(__file__))
        
        for locale, translations in self.translations.items():
            locale_file = os.path.join(locales_dir, f"{locale}.json")
            try:
                with open(locale_file, "w", encoding="utf-8") as f:
                    json.dump(translations, f, ensure_ascii=False, indent=4)
            except Exception as e:
                logging.error(f"Error saving translations for {locale}: {e}")

# Creating global instance of translation manager
translation_manager = TranslationManager()

def tr(text: str) -> str:
    """Helper function for translating text"""
    return translation_manager.translate(text) 