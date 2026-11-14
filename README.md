# Lichii NX Explorer - Android App

A minimal, modern Android app for exploring NX game archives (used by the Eden/Lichii Nintendo Switch emulator).

## Features

- **Modern Material 3 UI** - Clean, responsive interface with dark/light theme support
- **NX Archive Support** - Browse and extract files from `.nx` game archives
- **Native Performance** - JNI bindings to the nxformat C++ reader library
- **File Association** - Opens `.nx` files directly from file managers
- **Asset Loading** - Load sample archives from app assets

## Architecture

```
app/
├── src/main/
│   ├── java/com/eden/lichii/nxformat/
│   │   ├── NxFormatNative.kt          # JNI interface
│   │   ├── MainActivity.kt            # Main activity
│   │   ├── model/
│   │   │   └── ArchiveState.kt        # ViewModel & state
│   │   └── ui/
│   │       ├── MainScreen.kt          # Compose UI
│   │       └── theme/                 # Material 3 theme
│   ├── cpp/
│   │   ├── nxformat_jni.cpp           # JNI bridge
│   │   ├── nxformat_wrapper.h/.cpp    # C++ wrapper for nxformat
│   │   └── CMakeLists.txt             # Native build config
│   ├── jniLibs/arm64-v8a/             # Prebuilt libnxformat.so (needs to be added)
│   └── res/                           # Resources
```

## Building

### Prerequisites

1. Android Studio Hedgehog or later
2. Android NDK r26+
3. CMake 3.22+
4. The nxformat C library (`libnxformat.so`) built for ARM64

### Getting libnxformat.so

The nxformat library needs to be built from source for Android ARM64:

```bash
# Clone nxformat
git clone https://github.com/fab918/nxformat.git
cd nxformat

# Build for Android ARM64 (requires Android NDK)
mkdir build-android && cd build-android
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-24 \
      -DCMAKE_BUILD_TYPE=Release \
      ..
cmake --build . --config Release

# Copy the library
cp libnxformat.so /path/to/LichiiAndroid/app/src/main/jniLibs/arm64-v8a/
```

### Building the App

1. Open in Android Studio
2. Sync Gradle
3. Build → Make Project
4. Run on device/emulator

## NX Format Overview

The NX format is a game archive format used by the Eden/Lichii emulator with:

- **Crash-safe updates** - Dual header slots with generation counters
- **Zstandard compression** - Per-block compression (1 MiB blocks)
- **Integrity verification** - CRC-32C per slice, SHA-256 for catalog/manifest
- **Media support** - Icons and artwork stored in media region
- **Hierarchical structure** - Directories and files with sorted entries

## JNI Interface

The `NxFormatNative` class provides:

```kotlin
// Open from file path
fun openArchive(path: String): Boolean

// Open from assets
fun openAsset(assetManager: AssetManager, assetName: String): Boolean

// Browse directory
fun listDirectory(handle: Long): List<DirectoryEntry>

// Get file info
fun getFileInfo(fileId: Long): FileInfo?

// Read file data
fun readFile(fileId: Long, offset: Long = 0, length: Int = -1): ByteArray?

// Archive metadata
fun getArchiveInfo(): ArchiveInfo?
```

## License

GPL-3.0-or-later (same as Eden/Lichii emulator)

The nxformat library is MIT licensed (Copyright 2026 fab918).

## Credits

- **nxformat library**: https://github.com/fab918/nxformat by fab918
- **Eden Emulator**: https://github.com/eden-emulator/eden
- **Lichii Frontend**: https://github.com/zortos293/Lichii
- **Material 3**: https://m3.material.io/