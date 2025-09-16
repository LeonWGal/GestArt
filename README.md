# 🎨 GestArt

<div align="center">

**A powerful PyQt6-based image viewer with timer functionality for gesture drawing practice, reference study, and casual image browsing**

[![Version](https://img.shields.io/badge/version-0.9.8-blue.svg)](https://github.com/yourusername/gestart/releases)
[![Python](https://img.shields.io/badge/python-3.8+-green.svg)](https://python.org)
[![PyQt6](https://img.shields.io/badge/PyQt6-6.4+-orange.svg)](https://pypi.org/project/PyQt6/)
[![License](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)
[![Languages](https://img.shields.io/badge/languages-100-brightgreen.svg)](#language-support)

[English](README.md) • [Русский](README.ru.md)

#### 🌟 Complete Language List (100 Languages)
**English**, **Русский**, Afrikaans, አማርኛ, العربية, Авар мацӏ, Azərbaycan, Башҡортса, Беларуская, Български, বাংলা, Буряад хэлэн, Català, Нохчийн мотт, Чӑваш чӗлхи, Čeština, Dansk, Дарган мез, Deutsch, Ελληνικά, Esperanto, Español, Eesti, فارسی, Fulfulde, Suomi, Français, ગુજરાતી, Hausa, עברית, हिन्दी, Hrvatski, Kreyòl ayisyen, Magyar, Հայերեն, Bahasa Indonesia, Igbo, Italiano, 日本語, Basa Jawa, ქართული, Адыгэбзэ, Қазақша, ខ្មែរ, ಕನ್ನಡ, 한국어, Къарачай тил, Къумукъ тил, Коми кыв, Кыргызча, Лезги чӏал, ລາວ, Lietuvių, Latviešu, Мокшень кяль, Malagasy, Марий йылме, മലയാളം, Монгол, मराठी, Bahasa Melayu, မြန်မာ, Эрзянь кель, नेपाली, Nederlands, Norsk, Afaan Oromoo, Ирон ӕвзаг, ਪੰਜਾਬੀ, Polski, Português, Português (Brasil), Runa Simi, Română, Саха тыла, සිංහල, Sindarin, Slovenčina, Slovenščina, Shqip, Српски, Svenska, Kiswahili, தமிழ், తెలుగు, ไทย, Türkmen, Tagalog, Türkçe, Татарча, Тоҷикӣ, Удмурт кыл, Українська, اردو, O'zbekcha, Tiếng Việt, Yorùbá, 中文 (简体), 中文 (繁體), IsiZulu

</div>

## ✨ Features

### 🎯 Core Functionality
- ⏱️ **Customizable timer** for image display with unlimited mode (1-900 seconds)
- 🖼️ **Multiple image formats** support (PNG, JPG, JPEG, BMP, GIF, WEBP, TIFF, ICO, SVG, HEIC, HEIF)
- 📁 **Recursive folder scanning** with progress indication
- 📚 **Viewed images history** with clear option and session tracking
- 🎨 **Image effects** (B/W filter, vertical/horizontal flip, 90° rotation)
- 🔍 **Image zoom and pan** with mouse control and keyboard shortcuts
- 🌐 **100 languages support** with complete interface translations

### ⚙️ Advanced Settings
- ⏸️ **Configurable breaks** between sessions (1-60 minutes)
- 💾 **Settings persistence** between launches
- 🎨 **9 beautiful themes** (System, Dark, Light, Calcite, Charoite, Emerald, Jasper, Ruby, Sapphire)
- 🎭 **Modern UI** with smooth transitions and animations
- 📍 **Customizable timer position** (Left, Center, Right)

### 🚀 Performance & UX
- 🖱️ **Drag and drop folder** support with preview mode
- 🖥️ **Full-screen support**
- ⚡ **Asynchronous image loading** with memory-efficient caching
- 🛡️ **Error handling** and automatic recovery
- 📊 **Folder statistics tracking**
- 📋 **Image copy to clipboard** functionality
- 🗑️ **Trash functionality** with confirmation dialog

### 🎵 Audio & Visual
- 🔊 **Timer sound effects** with volume control
- 📐 **Customizable grid** with dual-color lines (1-3 horizontal/vertical lines)
- 🎨 **System theme detection** and integration
- 🔔 **Session completion notifications**
- ⏰ **Break timer** with skip option
- ⬅️➡️ **Navigation history** with back/forward support across sessions


## 📋 Requirements

| Component | Version | Description |
|-----------|---------|-------------|
| **Python** | 3.8+ | Core runtime (tested with 3.8-3.12) |
| **PyQt6** | 6.4.0+ | GUI framework |
| **send2trash** | 1.8.0+ | Safe file deletion |

## 🚀 Quick Start

### 📥 Installation

#### Option 1: Clone Repository
```bash
# Clone the repository
git clone https://github.com/yourusername/gestart.git
cd gestart

# Install dependencies
pip install -r requirements.txt
```

#### Option 2: Direct Download
1. Download the latest release from [Releases](https://github.com/yourusername/gestart/releases)
2. Extract the archive
3. Install dependencies:
```bash
pip install PyQt6 send2trash
```

### ▶️ Running the Application

```bash
python gestart.py
```

> 💡 **Tip**: On Windows, you can also double-click `gestart.py` to run the application.

## ⌨️ Controls

### 🎮 Keyboard Shortcuts

| Key | Action | Description |
|-----|--------|-------------|
| `←/→` | **Navigate** | Previous/Next image |
| `Space` | **Control** | Pause/Resume or Start new session |
| `S` | **Skip** | Skip current image |
| `O` | **Open** | Open file in system viewer |
| `C` | **Copy** | Copy image to clipboard |
| `G` | **Grid** | Toggle grid overlay |
| `Delete` | **Delete** | Move to trash |
| `B` | **Filter** | Black & White filter |
| `V` | **Flip** | Flip vertically |
| `H` | **Flip** | Flip horizontally |
| `R` | **Rotate** | Rotate 90° |
| `Backspace` | **Reset** | Reset image transformations |
| `T` | **Timer** | Show/Hide timer |
| `A` | **Always on Top** | Toggle always on top |
| `Ctrl+,` | **Settings** | Open settings dialog |
| `Ctrl++` | **Zoom In** | Increase zoom level |
| `Ctrl+-` | **Zoom Out** | Decrease zoom level |
| `Ctrl+0` | **Reset Zoom** | Reset to original size |

### 🖱️ Mouse Controls

| Action | Description |
|--------|-------------|
| **Left Click + Drag** | Pan image when zoomed |
| **Mouse Wheel** | Zoom in/out at cursor position |
| **Right Click** | Context menu (if available) |

## ⚙️ Settings & Configuration

### 📁 Data Storage

All settings and data are stored in the `~/.gestart/` folder:

| File | Description |
|------|-------------|
| `settings.json` | Application settings and preferences |
| `history.json` | Viewed images history and sessions |
| `folder_stats.json` | Folder viewing statistics and analytics |

### 🎛️ Available Settings

#### ⏱️ Session Settings
- **Display time per image**: 1-900 seconds (with unlimited option)
- **Number of images per session**: 1-900 images (with unlimited option)  
- **Break duration**: 1-60 minutes between sessions
- **Timer sound volume**: 0-100% audio control

#### 🎨 Interface Settings
- **Theme selection**: 9 beautiful themes (System, Dark, Light, Calcite, Charoite, Emerald, Jasper, Ruby, Sapphire)
- **Language selection**: 100 languages with instant switching
- **Timer position**: Left, Center, or Right alignment
- **Grid settings**: 1-3 horizontal/vertical lines with dual-color support
- **Always on top**: Keep window above other applications

#### 📚 History & Files
- **History saving**: Automatic tracking of viewed images
- **File deletion confirmation**: Safety prompts for file operations
- **Preview mode**: Enhanced folder selection experience

#### 🚀 Advanced Features
- **Break mode**: Configurable session breaks
- **System theme detection**: Automatic theme adaptation
- **Folder statistics tracking**: Detailed usage analytics

## 🎨 About GestArt

**GestArt** is a powerful image viewer designed specifically for artists to practice gesture drawing, study reference poses, and browse image collections with precision timing.

### 💡 The Story Behind GestArt

For years, I used another program for gesture drawing practice but was dissatisfied with its limitations. As someone who doesn't code, I spent months working with AI to bring this vision to life. After many iterations and challenges, we've created a robust, feature-rich application that's now completely open-source and free for everyone to use, modify, and distribute.

### 👥 Credits & Acknowledgments

| Role | Contributor | Description |
|------|-------------|-------------|
| **🎨 Concept & Design** | LeonWGal | Original idea and user experience design |
| **💻 Development** | Cursor (AI) | Code implementation and technical development |
| **🔊 Audio** | Mixkit (Website) | Countdown sound effects and audio assets |
| **🎯 Icons** | Tabler (Website) | UI icons and visual elements |

### 🤝 Contributing

We welcome contributions from the community! Whether you're fixing bugs, adding features, or improving translations, your help makes GestArt better for everyone.

### 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📁 Project Structure

```
gestart/
├── 📄 gestart.py              # Main application file
├── 📋 requirements.txt        # Python dependencies
├── 📖 README.md              # English documentation
├── 📖 README.ru.md           # Russian documentation
├── 📄 LICENSE                # MIT License
├── 🌍 locales/               # Localization files (100 languages)
│   ├── en.json              # English translations
│   ├── ru.json              # Russian translations
│   ├── ja.json              # Japanese translations
│   ├── zh-CN.json           # Chinese (Simplified) translations
│   ├── ar.json              # Arabic translations
│   ├── ...                  # 95+ more language files
│   └── translations.py       # Translation system
├── 🎨 themes/                # Theme files
│   ├── thememanager.py       # Theme management system
│   ├── dark.py              # Dark theme
│   ├── light.py             # Light theme
│   ├── calcite.py           # Calcite theme
│   ├── charoite.py          # Charoite theme
│   ├── emerald.py           # Emerald theme
│   ├── jasper.py            # Jasper theme
│   ├── ruby.py              # Ruby theme
│   └── sapphire.py          # Sapphire theme
└── 🎵 resources/             # Application resources
    ├── *.png                # UI icons and graphics
    ├── *.wav                # Sound effects and audio
    └── *.ico                # Application icons
```

---

<div align="center">

**Made with ❤️ for artists worldwide**

[![GitHub](https://img.shields.io/badge/GitHub-Repository-black?logo=github)](https://github.com/yourusername/gestart)
[![Issues](https://img.shields.io/badge/Issues-Report%20Bug-red?logo=github)](https://github.com/yourusername/gestart/issues)
[![Pull Requests](https://img.shields.io/badge/PRs-Welcome-green?logo=github)](https://github.com/yourusername/gestart/pulls)
[![Stars](https://img.shields.io/badge/Stars-⭐-yellow?logo=github)](https://github.com/yourusername/gestart/stargazers)

</div> 
