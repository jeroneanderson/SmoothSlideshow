# SmoothSlideshow C++

A native C++ Qt implementation of a smooth slideshow viewer. Originally designed for high efficiency on Raspberry Pi, now ported to run natively on macOS (Apple Silicon & Intel) and Linux.

## Features
*   **Lightweight**: Built with Qt5 and C++ for minimal overhead.
*   **Customizable**: Configurable slide duration, transition speed, and loop settings.
*   **Cross-Platform**: Runs on Raspberry Pi OS (Debian) and macOS.

---

##  macOS Build Instructions

### 1. Prerequisites
You need Homebrew and Xcode Command Line Tools.

```bash
xcode-select --install
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Install the dependencies:
```bash
brew install cmake qt@5
```

### 2. Configure & Compile
Run the following from the project root. Note that we must link Homebrew's Qt5 explicitly.

```bash
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=$(brew --prefix qt@5) ..
make
```

### 3. Deploy & Sign (Important!)
To run as a standalone app on macOS (especially M1/M2/M3), you must bundle the Qt libraries and re-sign the application to prevent "Corrupt App" or "Code Signature Invalid" errors.

```bash
# 1. Bundle Qt libraries into the .app
$(brew --prefix qt@5)/bin/macdeployqt SmoothSlideshow.app

# 2. Re-sign the bundle (Required for Apple Silicon)
codesign --force --deep --sign - SmoothSlideshow.app
```

### 4. Run
```bash
open SmoothSlideshow.app
```
*Note: If macOS prevents opening due to "Unidentified Developer," right-click the app in Finder and select Open.*

---

## 🍓 Raspberry Pi Build Instructions

### 1. Prerequisites
```bash
sudo apt update
sudo apt install build-essential cmake qtbase5-dev qt5-default
```
*(Note: On newer Debian versions, `qt5-default` may be replaced by `qtbase5-dev` alone).*

### 2. Compile
```bash
mkdir build
cd build
cmake ..
make
```

### 3. Run
```bash
./SmoothSlideshow
```

---

## ⚙️ Configuration
The application saves settings automatically when you change them in the UI. You can also manually edit the configuration file.

**Location:**
*   macOS: `~/.config/Endless_Slides/config.json`
*   Linux/Pi: `~/.config/Endless_Slides/config.json`

**Configuration Keys:**
Based on `src/ConfigManager.cpp`:
```json
{
    "last_folder": "/path/to/your/images",
    "recursive": true,            // Search subfolders
    "slide_duration": 5.0,        // Seconds per slide
    "transition_time": 1.0,       // Crossfade duration
    "random_order": false,        // Shuffle images
    "continuous_loop": true,      // Loop back to start after last image
    "cache_max_size_mb": 512.0    // Max thumbnail cache size
}
```

---

## 🛠 Troubleshooting

*   **"Select Folder" button does not open on Mac**: Qt sometimes struggles with the native macOS Finder dialog due to sandboxing.
    *   *Fix*: Ensure your `MainWindow.cpp` uses the `QFileDialog::DontUseNativeDialog` flag in the `selectFolder()` function.

*   **App crashes immediately on Mac**: This is usually a code signing issue.
    *   *Fix*: Run the `codesign` command listed in the Build Instructions above.

*   **Build fails on Pi with "qt5-default not found"**: Newer Raspberry Pi OS versions (Bullseye/Bookworm) have removed this package.
    *   *Fix*: Just install `qtbase5-dev` and `qtbase5-dev-tools`.
