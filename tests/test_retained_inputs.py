"""Recover pinned transport inputs without trusting archive paths or stale pins."""
from concurrent.futures import ThreadPoolExecutor
import hashlib
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import threading
from unittest.mock import patch
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from package_retained_inputs import digest, restore_archive
import package_retained_inputs as retained


class RetainedInputTests(unittest.TestCase):
    def test_concurrent_archive_downloads_and_rejected_replacements(self):
        for replacement in (None, "corrupt", "symlink"):
            with self.subTest(replacement=replacement), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                (root / "packaging").mkdir()
                data = b"pinned input"
                expected = hashlib.sha256(data).hexdigest()
                contents = io.BytesIO()
                with tarfile.open(fileobj=contents, mode="w:gz") as package:
                    for name, body in [("build-inputs.json", json.dumps(dict(files={"inputs/tool.tar": expected})).encode()),
                                       ("inputs/tool.tar", data)]:
                        entry = tarfile.TarInfo(name); entry.size = len(body)
                        package.addfile(entry, io.BytesIO(body))
                archive_data = contents.getvalue()
                checksum = hashlib.sha256(archive_data).hexdigest()
                (root / "packaging/retained-inputs.json").write_text(json.dumps(dict(linux=dict(url="https://example.org/archive", sha256=checksum))))
                cache = root / "cache"
                archive = cache / ("retained-linux-" + checksum[:12] + ".tar.gz")
                records = [dict(filename="tool.tar", sha256=expected)]
                barrier = threading.Barrier(2, timeout=5)
                link = retained.os.link
                def publish(source, target):
                    if target == archive:
                        if replacement is None:
                            barrier.wait()
                        elif replacement == "corrupt":
                            target.write_bytes(b"wrong checksum")
                        else:
                            original = root / "original"
                            original.write_bytes(archive_data)
                            target.symlink_to(original)
                    return link(source, target)
                def response(*args, **kwargs):
                    result = io.BytesIO(archive_data)
                    result.geturl = lambda: "https://example.org/archive"
                    return result
                with patch.object(retained, "ROOT", root), patch.object(retained.urllib.request, "urlopen", side_effect=response) as network, patch.object(retained.os, "link", side_effect=publish):
                    if replacement is None:
                        with ThreadPoolExecutor(max_workers=2) as workers:
                            futures = [workers.submit(retained.restore, "linux", cache, records) for _ in range(2)]
                            for future in futures:
                                future.result(timeout=10)
                        self.assertEqual(network.call_count, 2)
                        self.assertEqual((cache / "tool.tar").read_bytes(), data)
                        self.assertEqual(digest(archive), checksum)
                    else:
                        with self.assertRaisesRegex(ValueError, "Concurrent retained archive"):
                            retained.restore("linux", cache, records)
                        self.assertFalse((cache / "tool.tar").exists())
                self.assertEqual(sorted(path.name for path in cache.iterdir()), sorted([archive.name] + (["tool.tar"] if replacement is None else [])))

    def test_restore_checks_archive_member_and_current_pin(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive, cache = root / "retained.tar.gz", root / "cache"
            data = b"original pinned binary"
            expected = hashlib.sha256(data).hexdigest()
            records = [dict(filename="tool.tar", sha256=expected), dict(filename="new.tar", sha256=expected)]
            def write(member_data):
                with tarfile.open(archive, "w:gz") as package:
                    for name, content in [("build-inputs.json", json.dumps(dict(files={"inputs/tool.tar": expected})).encode()),
                                          ("inputs/tool.tar", member_data), ("inputs/../../escape", b"unselected")]:
                        entry = tarfile.TarInfo(name); entry.size = len(content)
                        package.addfile(entry, io.BytesIO(content))
            write(data)
            self.assertEqual(restore_archive(archive, cache, records, digest(archive)), 1)
            self.assertEqual((cache / "tool.tar").read_bytes(), data)
            self.assertFalse((cache / "new.tar").exists())
            self.assertFalse((root / "escape").exists())
            self.assertEqual(restore_archive(archive, cache, records, digest(archive)), 0)
            with self.assertRaisesRegex(ValueError, "archive checksum"):
                restore_archive(archive, cache, records, "0" * 64)
            (cache / "tool.tar").unlink()
            changed_pin = [dict(filename="tool.tar", sha256="1" * 64)]
            self.assertEqual(restore_archive(archive, cache, changed_pin, digest(archive)), 0)
            write(b"changed binary with stale inventory")
            with self.assertRaisesRegex(ValueError, "member checksum"):
                restore_archive(archive, cache, records, digest(archive))
            self.assertEqual(list(cache.iterdir()), [])
            with self.assertRaisesRegex(ValueError, "Invalid retained input filename"):
                restore_archive(archive, cache, [dict(filename="../escape", sha256=expected)], digest(archive))


if __name__ == "__main__":
    unittest.main()
