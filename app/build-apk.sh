#!/bin/bash
# Fallback: build the Minecraft PE 0.6.1 debug APK WITHOUT Gradle.
#
# Used only when ./gradlew cannot run in CI (the Gradle distribution
# download is the fragile part). Same direct pipeline as the old
# repo's build-apk.sh (proven in CI there): ndk-build + aapt2/javac/d8
# + zipalign/apksigner. Adapted to this repo's standard Gradle layout
# (app/src/main/{jni,java,res,assets}, game sources at repo root src/,
# RakNet at repo root raknet/).
#
# Env: ANDROID_HOME (SDK, required), NDK_R17C (NDK r17c dir; default
#   $ANDROID_HOME/ndk/17.2.4988734), BUILD_TOOLS_VERSION (default
#   36.0.0), JAVA_HOME (for javac/apksigner; default: java on PATH).
# Usage: build-apk.sh [output-apk-path]
set -e
cd "$(dirname "$0")"
SDK="${ANDROID_HOME:?ANDROID_HOME must be set}"
NDK="${NDK_R17C:-$SDK/ndk/17.2.4988734}"
BT_VER="${BUILD_TOOLS_VERSION:-36.0.0}"
# NOTE: build-tools 36.0.0's d8 (R8 8.10) is required — 34.0.0's d8
# (R8 8.2.2) crashes with an NPE while writing the dex for these
# classes ("String.length() because <parameter1> is null").
BT="$SDK/build-tools/$BT_VER"
AAPT2="$BT/aapt2"
ZIPALIGN="$BT/zipalign"
APKSIGNER="$BT/apksigner"
D8="$BT/d8"
JAVAC="${JAVA_HOME:+$JAVA_HOME/bin/}javac"
JAR="${JAVA_HOME:+$JAVA_HOME/bin/}jar"
OUT="${1:-out/minecraft-mcpe-android-0.6.1-debug.apk}"
STAGE="out/apk-stage"
ANDROID_JAR="$(ls -d "$SDK"/platforms/android-*/android.jar 2>/dev/null | sort -V | tail -1)"

[ -x "$NDK/ndk-build" ] || { echo "NDK r17c not found at $NDK (set NDK_R17C)"; exit 1; }
[ -f "$ANDROID_JAR" ] || { echo "no android.jar under $SDK/platforms"; exit 1; }
[ -x "$AAPT2" ] || { echo "aapt2 not found at $AAPT2 (set BUILD_TOOLS_VERSION)"; exit 1; }

rm -rf "$STAGE"
mkdir -p "$STAGE/lib/arm64-v8a" "$STAGE/lib/armeabi-v7a" "$STAGE/gen" "$STAGE/classes" "$STAGE/dex"

# 0. NDK r17c's clang links libtinfo.so.5, which modern distros
#    (and GitHub runners) no longer ship — only libtinfo.so.6.
#    A symlink to .so.6 is enough for the compiler driver.
if ! ldconfig -p 2>/dev/null | grep -q "libtinfo.so.5"; then
    TINFO6="$(ldconfig -p 2>/dev/null | grep "libtinfo.so.6" | head -1 | awk '{print $NF}')"
    if [ -n "$TINFO6" ]; then
        mkdir -p "$STAGE/libtinfo5"
        ln -sf "$TINFO6" "$STAGE/libtinfo5/libtinfo.so.5"
        export LD_LIBRARY_PATH="$PWD/$STAGE/libtinfo5${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
        echo "libtinfo.so.5 shim: $TINFO6"
    fi
fi

echo "== 1. native libs (NDK r17c: $NDK) =="
"$NDK/ndk-build" NDK_PROJECT_PATH="$PWD" \
    APP_BUILD_SCRIPT="$PWD/src/main/jni/Android.mk" \
    NDK_APPLICATION_MK="$PWD/src/main/jni/Application.mk" \
    NDK_MODULE_PATH="$PWD/.." \
    APP_ABI="arm64-v8a armeabi-v7a" -j"$(nproc)"
cp libs/arm64-v8a/libminecraftpe.so "$STAGE/lib/arm64-v8a/"
cp libs/armeabi-v7a/libminecraftpe.so "$STAGE/lib/armeabi-v7a/"

echo "== 2. compile resources (aapt2) =="
"$AAPT2" compile --dir src/main/res -o "$STAGE/res.zip"

echo "== 3. link manifest + resources + assets =="
"$AAPT2" link --min-sdk-version 21 --target-sdk-version 30 \
    --version-code 40 --version-name "0.6.1" \
    --auto-add-overlay \
    -I "$ANDROID_JAR" \
    --manifest src/main/AndroidManifest.xml \
    -R "$STAGE/res.zip" -A src/main/assets \
    --java "$STAGE/gen" --custom-package com.mojang.minecraftpe \
    --rename-manifest-package com.custom.minecraftpe \
    -o "$STAGE/unsigned.apk"

echo "== 4. Java (javac + d8) =="
# app/build.gradle excludes Minecraft_Verizon.java; keep everything else.
find src/main/java -name "*.java" ! -name "Minecraft_Verizon.java" > "$STAGE/sources.txt"
"$JAVAC" -nowarn -source 8 -target 8 -bootclasspath "$ANDROID_JAR" \
    -cp "$STAGE/gen" -d "$STAGE/classes" @"$STAGE/sources.txt"
# NOTE: d8 (build-tools 34/36) rejects a bare classes directory as
# program input ("Unsupported source file type") — jar it first.
(cd "$STAGE/classes" && "$JAR" cf ../classes.jar .)
"$D8" --release --lib "$ANDROID_JAR" --output "$STAGE/dex" "$STAGE/classes.jar"

echo "== 5. add dex + native libs (stored, uncompressed) =="
python3 - "$STAGE" <<'EOF'
import sys, zipfile
stage = sys.argv[1]
with zipfile.ZipFile(stage + "/unsigned.apk", "a", zipfile.ZIP_STORED) as z:
    z.write(stage + "/dex/classes.dex", "classes.dex")
    z.write(stage + "/lib/arm64-v8a/libminecraftpe.so",
            "lib/arm64-v8a/libminecraftpe.so")
    z.write(stage + "/lib/armeabi-v7a/libminecraftpe.so",
            "lib/armeabi-v7a/libminecraftpe.so")
EOF

echo "== 6. align + sign (debug keystore) =="
# The output path may live under app/build/outputs/... which Gradle
# never created when gradlew failed — apksigner does not create
# parent directories, so make it here (CI run 42483450427 died on
# exactly this: FileNotFoundException for the --out path).
mkdir -p "$(dirname "$OUT")"
"$ZIPALIGN" -f -p 4 "$STAGE/unsigned.apk" "$STAGE/aligned.apk"
if [ ! -f "$STAGE/debug.keystore" ]; then
    keytool -genkeypair -keystore "$STAGE/debug.keystore" \
        -alias androiddebugkey -storepass android -keypass android \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -dname "CN=Android Debug,O=Android,C=US" -noprompt
fi
"$APKSIGNER" sign --ks "$STAGE/debug.keystore" --ks-pass pass:android \
    --key-pass pass:android --ks-key-alias androiddebugkey \
    --out "$OUT" "$STAGE/aligned.apk"
echo "APK: $OUT"
ls -la "$OUT"
