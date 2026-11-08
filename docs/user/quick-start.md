# Quick Start

[Documentation](../README.md) · [Русский](ru/quick-start.md) · [User guide](guide.md)

Transcribe v0.5.0 recognizes Russian speech locally on CPU. The GUI supports
Russian and English; CLI console output remains Russian.

## Download

Get the package for your OS from [v0.5.0](https://github.com/khdobromir/transcriber/releases/tag/v0.5.0):

- [Linux AppImage](https://github.com/khdobromir/transcriber/releases/download/v0.5.0/Transcribe-0.5.0-linux-x86_64.AppImage):
  x86_64, glibc 2.39+.
- [Windows ZIP](https://github.com/khdobromir/transcriber/releases/download/v0.5.0/Transcribe-0.5.0-windows-x64.zip):
  Windows 11 x64.

Models are separate downloads. Source and build-input archives are for developers.
Neither binary package needs a separate Qt SDK, Python, compiler or media-tool installation.

## Verify and launch

Compare SHA-256 with the digest displayed for the **same asset** on the
GitHub Release page (expand its details). No separate checksum files are published.

Linux:

```sh
sha256sum Transcribe-0.5.0-linux-x86_64.AppImage
chmod +x Transcribe-0.5.0-linux-x86_64.AppImage
./Transcribe-0.5.0-linux-x86_64.AppImage
```

If FUSE is unavailable, run the same file with the extraction option;
temporary disk space is required:

```sh
./Transcribe-0.5.0-linux-x86_64.AppImage --appimage-extract-and-run
```

Windows PowerShell:

```powershell
Get-FileHash -Algorithm SHA256 -LiteralPath '.\Transcribe-0.5.0-windows-x64.zip'
```

After verification, extract the **entire ZIP**, preserve its directories and run
`bin\transcribe-gui.exe`. Keep its bundled tools together.

## Prepare models and transcribe

1. On **Transcription / Расшифровка**, select `small` for an initial test.
   A fresh GUI defaults to `medium`, which may take longer.
2. Open **Models / Модели** and choose **Download small and VAD / Скачать small и VAD**.
   Wait for successful verification. For an offline first run, obtain the exact
   recognition and VAD files listed in the [model manifest](../../scripts/models.tsv)
   beforehand, then use **Import small… / Импорт small…** and **Import VAD… / Импорт VAD…**.
   Match the selected preset before importing.
3. Return to **Transcription / Расшифровка**, select your local speech recording
   and an output directory. Keep one chunk and automatic CPU settings, then
   choose **Start transcription / Начать расшифровку**.
4. Wait for **Completed / Готово**, then open **Directory / Каталог** or **Full TXT / Полный TXT**.
   Each run has `transcripts/transcript.txt`, `.srt`, `.vtt`, `logs/`, `source.txt`
   and `result.json`. Preview text and 100% progress alone do not confirm saved exports.

The language selector under **Settings / Настройки** offers
**Automatic / Автоматически**, **Русский** and **English**. Automatic chooses
Russian on Russian systems and English otherwise. Selection is saved and switching
takes effect immediately, preserving current work. Recognition and CLI language
remain Russian; native OS dialogs may keep their system language.

After model preparation, local-file recognition needs no network. URL and model
downloads do. **Cancel / Отменить** preserves published partial text; retry starts
recognition again. See [troubleshooting](../troubleshooting.md) for missing models,
download failures or incomplete packages.

CLI use is separate: the Windows ZIP contains `bin\transcribe.exe`; the Linux
AppImage does not install a command in PATH. See the
[CLI quick start](../../README.en.md#cli-quick-start),
[source installer prerequisites](../build/from-source.md#linux-cli-source-installer)
and [CLI reference](cli.md).
