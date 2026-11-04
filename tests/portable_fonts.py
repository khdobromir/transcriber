"""Shared assertions for actual portable startup; no harness can substitute for AppRun."""
import json
from pathlib import Path
import re
import subprocess


def require_clean_diagnostics(path):
    text = Path(path).read_text(errors="replace")
    if re.search(r"Fontconfig\s+(?:warning|error)\b|failed to load|is not installed|TypeError|ReferenceError|Binding loop", text, re.I):
        raise RuntimeError("Portable startup diagnostics failed; full output saved in " + str(path))


def require_font_report(path):
    report = json.loads(Path(path).read_text())
    for key in ['portable_fontconfig', 'bundled_fontconfig', 'fontconfig_manifest_match', 'cyrillic']:
        if report.get(key) is not True:
            raise RuntimeError("Portable font check failed: " + key + "; report: " + str(path))
    root = Path(report['bundle_root'])
    library = Path(report['fontconfig_library'])
    if not library.is_relative_to(root / 'usr/lib') or report['config'] != str(root / 'usr/share/transcribe/fonts.conf'):
        raise RuntimeError("Fontconfig did not use the private bundle paths")
    if not re.fullmatch(r'[0-9a-f]{64}', report.get('fontconfig_sha256', '')) or report.get('fontconfig_version', 0) <= 0:
        raise RuntimeError("Missing loaded Fontconfig identity")
    return report


def check_broken_config(root, env, artifacts):
    config = root / 'usr/share/transcribe/fonts.conf'
    original = config.read_bytes()
    log = artifacts / 'damaged-fontconfig.log'
    try:
        config.write_bytes(b'<fontconfig><broken>')
        child_env = dict(env, TRANSCRIBE_PORTABLE_DIAGNOSTICS=str(artifacts / 'damaged-fontconfig.json'))
        with log.open('wb') as output:
            result = subprocess.run([str(root / 'AppRun')], env=child_env, stdout=output,
                                    stderr=subprocess.STDOUT, timeout=20, check=False)
        if result.returncode == 0 or b'Portable Fontconfig initialization failed' not in log.read_bytes():
            raise RuntimeError('Damaged font config did not fail at application startup; see ' + str(log))
    finally:
        config.write_bytes(original)
