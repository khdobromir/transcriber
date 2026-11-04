"""Exercise private font startup in isolated processes, including a skipped initializer."""
import json
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

probe, library, config, font = map(Path, sys.argv[1:])
with tempfile.TemporaryDirectory(prefix="transcribe-fonts-") as temporary:
    root = Path(temporary) / "Комплект 😀"
    tools = root / "usr/bin/tools"
    tools.mkdir(parents=True)
    (tools / ".transcribe-bundle").write_text("1\n")
    libraries = root / "usr/lib"
    libraries.mkdir()
    shutil.copy2(library.resolve(), libraries / "libfontconfig.so.1")
    (root / "package-manifest.json").write_text(json.dumps({'files': [
        {'path': 'usr/lib/libfontconfig.so.1', 'sha256': hashlib.sha256(library.resolve().read_bytes()).hexdigest()}]}))
    private = root / "usr/share/transcribe/fonts.conf"
    private.parent.mkdir(parents=True)
    shutil.copy2(config, private)
    fonts = root / "usr/share/fonts"
    fonts.mkdir()
    shutil.copy2(font, fonts / font.name)
    output = Path(temporary) / "fonts.json"
    env = dict(os.environ, TRANSCRIBE_PORTABLE_ROOT=str(root), TRANSCRIBE_PORTABLE_DIAGNOSTICS=str(output),
               LD_LIBRARY_PATH=str(libraries), QT_QPA_PLATFORM="offscreen", QT_QPA_PLATFORMTHEME="generic",
               QT_QPA_FONTDIR=str(fonts),
               XDG_CACHE_HOME=temporary + "/cache", XDG_CONFIG_HOME=temporary + "/config")
    result = subprocess.run([str(probe)], env=env, capture_output=True, timeout=30)
    assert result.returncode == 0, (result.stderr.decode(errors="replace")
                                    + "\nFont report: " + (output.read_text() if output.exists() else "missing"))
    assert b"Fontconfig warning" not in result.stderr and b"Fontconfig error" not in result.stderr, result.stderr
    report = json.loads(output.read_text())
    assert report["portable_fontconfig"] and report["bundled_fontconfig"] and report["cyrillic"], report
    assert Path(report["fontconfig_library"]).resolve().is_relative_to(libraries.resolve()), report
    result = subprocess.run([str(probe), "--skip-init"], env=env, capture_output=True, timeout=30)
    assert result.returncode != 0 and b"require portable initialization" in result.stderr, result.stderr
    for data in [None, b"<fontconfig><broken>"]:
        if data is None:
            private.unlink()
        else:
            private.write_bytes(data)
        result = subprocess.run([str(probe)], env=env, capture_output=True, timeout=30)
        assert result.returncode != 0 and b"Portable Fontconfig initialization failed" in result.stderr, result.stderr
print("PASS private Fontconfig initialization, library identity, Cyrillic, skipped init and damaged config")
