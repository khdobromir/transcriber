# Transcription example

[Русский README](../../../README.md) · [English README](../../../README.en.md)

These are unmodified exports from the final Transcribe v0.5.0 Linux release
verification. They illustrate the output formats, not a reference transcript,
WER measurement or performance benchmark.

- [Full TXT](transcript.txt)
- [SRT subtitles](transcript.srt)
- [WebVTT subtitles](transcript.vtt)
- [Source, parameters and SHA-256 records](provenance.json)

## Recording and attribution

The input is the 30–60 second excerpt of
[Ru-Russian language (intro).ogg](https://commons.wikimedia.org/w/index.php?title=File:Ru-Russian_language_(intro).ogg&oldid=1157054605),
converted to mono PCM s16le WAV at 16 kHz with metadata removed.
The excerpt itself lasts 30 seconds; exported subtitle times start at zero.

Attribution: Andrew Krizhanovsky and Nataly Lugovaya (voices);
Andrei Rybak (noise reduction); Russian Wikipedia article authors (text).
Source material is licensed under [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/).
The exports contain an automatic transcription of the excerpt and retain its
attribution and license. The input audio is not bundled here.

## Recognition parameters

| Parameter | Value |
| --- | --- |
| Application | Transcribe 0.5.0, source `b8ebf6e1655b8550358f1a54fc70589b02f67899` |
| Package | Published Linux x86_64 AppImage; SHA-256 in `provenance.json` |
| Model | `small-q5_1` (the `small` preset) |
| Recognition language | Russian |
| CPU threads / chunks / workers | 2 / 1 / 1 |
| VAD | Disabled for this recorded verification run |
| Completion | Completed, exit code 0; all three exports present |

The [release evidence](https://github.com/khdobromir/transcriber/releases/download/v0.5.0/release-validation.json)
identifies the final package. Input WAV and output hashes were checked against
the retained release-verification record before these files were copied.
See the [fixture manifest](../../../tests/fixtures/russian-speech.json) for
the original audio URL, hashes, license and conversion details.

## GUI image

The [result screenshot](../transcription.png) shows this completed result opened
from history in the actual v0.5.0 AppImage. The screenshot uses a copy of the
result with a neutral title and temporary paths, isolated GUI settings and an
empty model directory. Export contents are unchanged; no recognition was run
while taking the screenshot. The selected preset is `small`.

The [Models screenshot](../gui.png) illustrates selection, download and import.
It does not establish that a model is installed or that recognition has completed.

---

Это подлинные TXT/SRT/VTT из проверки финального релиза v0.5.0.
Запись публичная; источник, авторы и лицензия CC BY 3.0 указаны выше.
Снимок GUI показывает копию готового результата из истории с временными путями.
Параметры сохранённого прогона: `small-q5_1`, два потока, одна часть,
один работник, VAD выключен. В быстром старте VAD остаётся включён по умолчанию.
Пример не измеряет точность или скорость распознавания.
