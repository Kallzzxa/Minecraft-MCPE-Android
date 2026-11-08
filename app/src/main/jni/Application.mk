APP_PLATFORM := android-21
# NOTE: was android-9; arm64-v8a requires API 21+ (NDK errors on
# arm64 with android-9). 32-bit ABIs also build fine against 21.
APP_STL := stlport_static
APP_OPTIM        := release
APP_ABI          := arm64-v8a armeabi-v7a
# NOTE: was all four ABIs; only arm64-v8a + armeabi-v7a are built
# (matches the Gradle abiFilters and the proven CI build).
