#!/bin/sh
# Run inside an ordinary minimal Ubuntu/Debian container, with only the final
# AppImage and optional public speech/model mounted. No build tree or Qt SDK.
set -eu
save_evidence() {
    status=$?
    if [ -d /evidence ]; then
        for file in /tmp/gui.log /tmp/cli.log /tmp/portable-fonts.json /tmp/fontconfig-extract.log /tmp/damaged-fontconfig.log /tmp/damaged-fontconfig.json; do
            if [ -f "$file" ]; then cp -- "$file" /evidence/ || status=1; fi
        done
        if [ -d /tmp/results ]; then cp -r /tmp/results /evidence/ || status=1; fi
    fi
    exit "$status"
}
verify_file() {
    expected=$1
    case "$expected" in *[!a-f0-9]*|'') printf 'Invalid SHA-256\n' >&2; exit 1;; esac
    test "${#expected}" -eq 64
    actual=$(sha256sum < "$2")
    test "${actual%% *}" = "$expected" || { printf 'SHA-256 mismatch: %s\n' "$2" >&2; exit 1; }
}
verify_inputs() {
    verify_file "${APPIMAGE_SHA256:?Missing AppImage hash}" "$1/Transcribe.AppImage"
    if [ "${REQUIRE_ASR:-0}" = 1 ]; then
        verify_file "${AUDIO_SHA256:?Missing audio hash}" "$1/audio.wav"
        verify_file "${MODEL_SHA256:?Missing model hash}" "$1/model.bin"
    fi
}
# Model-free regression entry point exercises the same verifier as the containers.
if [ "${1:-}" = --verify-inputs ]; then verify_inputs "$2"; exit 0; fi
trap save_evidence 0
for tool in ffmpeg ffprobe yt-dlp whisper-cli python3 qmake qmake6; do
    if command -v "$tool" >/dev/null 2>&1; then
        printf 'Unexpected installed application dependency: %s\n' "$tool" >&2
        exit 1
    fi
done
if dpkg-query -W 'libqt6core6*' 2>/dev/null; then
    printf 'Unexpected installed Qt runtime\n' >&2
    exit 1
fi
test -x /inputs/Transcribe.AppImage
verify_inputs /inputs
if [ "${REQUIRE_ASR:-0}" = 1 ]; then test -s /inputs/audio.wav && test -s /inputs/model.bin; fi
export TRANSCRIBE_HOME=/tmp/transcribe-data
export TRANSCRIBE_GUI_SETTINGS_FILE=/tmp/settings.ini
export TRANSCRIBE_PORTABLE_DIAGNOSTICS=/tmp/portable-fonts.json
export XDG_CACHE_HOME=/tmp/transcribe-xdg-cache XDG_CONFIG_HOME=/tmp/transcribe-xdg-config
export QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=generic QT_QUICK_BACKEND=software
unset LD_LIBRARY_PATH QT_PLUGIN_PATH QML_IMPORT_PATH QML2_IMPORT_PATH APPDIR APPIMAGE APPIMAGE_EXTRACT_AND_RUN
/inputs/Transcribe.AppImage --appimage-extract-and-run --transcribe-cli --version
gui_code=0
timeout --signal=TERM --kill-after=5s 10s /inputs/Transcribe.AppImage --appimage-extract-and-run > /tmp/gui.log 2>&1 || gui_code=$?
if [ "$gui_code" -ne 124 ]; then cat /tmp/gui.log; exit 1; fi
if grep -E 'Fontconfig[[:space:]]+(warning|error)|failed to load|is not installed|TypeError|ReferenceError|Binding loop' /tmp/gui.log; then exit 1; fi
if [ -f /inputs/audio.wav ]; then
    /inputs/Transcribe.AppImage --appimage-extract-and-run --transcribe-cli \
        --model /inputs/model.bin --no-vad --threads 2 --out /tmp/results -- /inputs/audio.wav > /tmp/cli.log 2>&1
    set -- /tmp/results/*
    test "$#" -eq 1 && test -d "$1"
    for extension in txt srt vtt; do test -s "$1/transcripts/transcript.$extension"; done
    grep '"status": "completed"' "$1/result.json" >/dev/null
fi
# These booleans are produced by the actual application after checking its
# loaded library against the package manifest and its font's Cyrillic glyphs.
for key in portable_fontconfig bundled_fontconfig fontconfig_manifest_match cyrillic; do
    grep -F "\"$key\":true" /tmp/portable-fonts.json >/dev/null
done
negative=/tmp/transcribe-damaged-fonts
mkdir -p "$negative"
(cd "$negative" && /inputs/Transcribe.AppImage --appimage-extract > /tmp/fontconfig-extract.log 2>&1)
printf '<fontconfig><broken>' > "$negative/squashfs-root/usr/share/transcribe/fonts.conf"
damaged_code=0
TRANSCRIBE_PORTABLE_DIAGNOSTICS=/tmp/damaged-fontconfig.json \
    timeout --signal=TERM --kill-after=5s 15s "$negative/squashfs-root/AppRun" > /tmp/damaged-fontconfig.log 2>&1 || damaged_code=$?
if [ "$damaged_code" -eq 0 ] || [ "$damaged_code" -eq 124 ] || [ "$damaged_code" -eq 137 ]; then
    cat /tmp/damaged-fontconfig.log
    exit 1
fi
grep -F 'Portable Fontconfig initialization failed' /tmp/damaged-fontconfig.log >/dev/null
printf 'Clean Linux package smoke passed (real font startup, Cyrillic and damaged config; no installed SDK/tools).\n'
