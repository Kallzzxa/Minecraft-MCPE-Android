# Minecraft-MCPE-Android

Android build of the Minecraft PE 0.6.1 "handheld" source (C++ + RakNet),
split out of [Kallzzxa/Minecraft-MCPE](https://github.com/Kallzzxa/Minecraft-MCPE)
into a standard Gradle project so it builds with the usual command:

```sh
./gradlew assembleDebug
```

The APK is `app/build/outputs/apk/debug/app-debug.apk`
(package `com.custom.minecraftpe`, version 0.6.1, arm64-v8a + armeabi-v7a).

## Layout

| Path | What it is |
|---|---|
| `app/` | Android app module (standard Gradle layout) |
| `app/src/main/` | Manifest, Java, JNI, assets, res |
| `src/` | MCPE C++ game sources (same tree as the old `handheld/src/`) |
| `raknet/` | RakNet static library sources |
| `app/build-apk.sh` | Fallback direct pipeline (ndk-build + aapt2/d8, no Gradle) |

Changes vs the upstream tree (all already proven in the old repo):
- `APP_PLATFORM` android-9 → 21 (arm64-v8a requires API 21+)
- Removed `-isystem $(SYSROOT)/usr/include` from the native build
  (pulled in host headers with NDK r17c)
- Case fixes: `RakNetSources/`, `EGLConfigPrinter.h`
- RakNet `FileList.cpp` skips `<asm/io.h>` / `<sys/io.h>` on Android
- Manifest: `android:exported="true"` (required for targetSdk 30)
- Right-side jump button in the Android touch controls
  (the 0.6.1 layout only had jump in the D-pad center)

## Requirements

- **JDK 8** — Gradle 4.10.1 / AGP 3.3.2 do not run on modern JDKs
- Android SDK: build-tools `30.0.3` + platform `android-30`
  (build-tools `36.0.0` + platform `android-35` are also used by the
  fallback pipeline's `d8`/`apksigner`)
- **NDK r17c** — the last NDK shipping `stlport_static`, which this
  tree needs (`APP_STL := stlport_static`). Set `ANDROID_NDK_HOME`
  (or `ndk.dir` in `local.properties`) to it.

## Build

```sh
./gradlew assembleDebug
```

### Note on the Gradle download

The wrapper downloads Gradle 4.10.1 from the GitHub Releases mirror
(`github.com/gradle/gradle-distributions`) — `services.gradle.org`
times out from GitHub runners, and `downloads.gradle-dn.com` does not
even resolve there. CI retries the wrapper against three mirrors in
turn (GitHub Releases, Aliyun, services.gradle.org). If `./gradlew` still
cannot run, CI falls back to the proven direct pipeline:

```sh
bash app/build-apk.sh app/build/outputs/apk/debug/app-debug.apk
```

## Known bugs (from the source tree)

Lighting is broken, frustum culling is partial (entities not 100%),
the Item/ItemInstance package is incomplete, quitting via the Back
button only hides the app instead of shutting down cleanly, and there
are a few static memory leaks. Render distance defaults to Short
(64 blocks) — change it in-game under Options → Video → Render
Distance → Far (256 blocks).

## Status

Build-only until tested on a real device. The first on-device run
(realme 8, Android 13, arm64) reached the in-game survival screen.
