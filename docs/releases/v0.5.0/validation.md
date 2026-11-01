# v0.5.0 local preparation and verification

Status: prepared locally for owner review, not published. The owner authorized
local commits and release preparation on 2026-11-01 and explicitly prohibited
pushes, tag creation and GitHub Release creation until review. No remote CI or
publication action is part of this preparation.

## Source and changes

The common CMake version is `0.5.0` with an empty prerelease suffix. The local
`release/v0.5.0` branch starts at beta master
`c7896644ec45f59b2365177406a42dcf72543c99`. UI fixes and acceptance evidence are
committed as `dbbbcc24b96ba006ab1048f2c1b824e38143b2e6`; the release-preparation
commit adds the stable version and documentation. Its exact SHA/tree and source
archive digest belong in the external local preparation record, avoiding a
self-referential source commit.

The change holds the bounded transcript preview and selection during reading,
resumes at the bottom or via a button, suppresses unchanged preview updates and
resets the view when another result is selected. Shared ETA sampling is limited
to five-second intervals, with time-based finish-time smoothing. GUI estimates
use rounded units, report stalls and clear on cancellation/finalization/termination.
The CLI arguments, JSONL v1 wire shape and result files remain compatible.

## Acceptance and local checks

The user reported successful Windows 11 beta acceptance, then successful focused
Windows verification of the transcript/ETA fixes. OS characteristics are the same
as the previous report: Windows 11 24H2. The complete OS build, tested package
filename/digest and exact source SHA were not supplied. Accept these as manual
user reports; do not invent artifact-level verification or require the user to
repeat an already accepted check merely to record this preparation.

Local UI implementation evidence on 2026-11-01 used Linux x86_64, GCC 16.2.1 and
Qt 6.12.0: all 28 CTest suites passed, including GUI controller, QML at 100/150/200%
scales, GUI-to-CLI integration, pipeline and packaging-helper contracts. The HTTP
fixture tests required execution outside the sandbox's local-socket restriction.
All 182 English translations passed extraction/placeholder checks; QML lint and
clang-format 18.1.8 passed. QML lint retained an unrelated unused import notice in
`HistoryPage.qml`.

ASan/UBSan passed the ETA units, GUI QML and GUI-to-CLI integration plus focused
controller completion/cancellation/stale-preview cases. Leak detection was
disabled for these runs; no LSan pass is claimed. clang-tidy passed the six
changed C++ translation units after removing the unsupported GCC-only
`-mno-direct-extern-access` flag from a temporary analysis database. Project
compiler flags and analysis checks were not weakened.

Fresh stable-version build/test/version/help/install results and compact logs
are retained with the local preparation record in
`dist/releases/v0.5.0-local-review/`. This directory is ignored by Git and is not
an uploaded release. No previous beta CI or package digest validates a new stable
package. Native MSVC, Qt 6.8.3 package builds and final-SHA remote CI are pending.

For stable `0.5.0`, GCC Debug rebuilt successfully and all 28 CTest suites passed
again. Clang 23.1.4 built the application-only Release GUI/CLI with warnings as
errors; temporary installation and installed CLI version/help checks passed
(`transcribe 0.5.0`). The local Linux install requires the host Qt/runtime and is
not an AppImage. Actual CLI recognition of the public five-second JFK fixture
with verified medium-q5_0 and Silero VAD 6.2.0 passed, producing TXT/SRT/VTT in
about 21 seconds. This is a technical ASR smoke, not Russian WER or portable
package acceptance. Installed user models/settings/results were not modified.

## Review and remaining delivery work

The scoped code review found no critical/major defects in the UI change. The
bounded held snapshot, generation checks and steady-clock tests cover the
reported failures without changing the protocol. The selected files contain
source, tests and documentation; no private media, models, cookies, generated
builds or editor settings are staged. The user's untracked `.vscode/` is preserved;
future release packaging must use a clean checkout of the committed candidate.

An existing Linux packaging issue remains: `packaging/linux/AppRun` exports the
bundle's `LD_LIBRARY_PATH`/Qt paths, while `QDesktopServices::openUrl()` launches
external applications without restoring their environment. Earlier beta
diagnosis reproduced a Nautilus library-symbol failure; the current source still
has that combination. Fontconfig host-rule compatibility was also unresolved.
This is a known distribution risk, not a passing external-desktop integration
check. Publication review must account for it; the UI commits do not fix it.

After owner review, use the existing release procedure to obtain final-SHA CI,
build the final Windows ZIP/Linux AppImage and matching source/build-input/notice
materials, and record actual hashes and skipped checks. Do not reuse or rename
beta binaries as stable. The canonical 12-asset contract remains defined by
`scripts/release_assets.py`; the locally prepared source archive alone does not
satisfy it. Create an annotated `v0.5.0` tag and publish only after authorization.
