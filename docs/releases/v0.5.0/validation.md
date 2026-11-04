# v0.5.0 verification scope and preparation history

Status: stable publication and local upgrade authorized by the owner on 2026-11-04.
The release asset `release-validation.json` records actual final master CI,
package delivery and publication results. The earlier local review restriction
has been superseded by this authorization. The canonical
repository is [khdobromir/transcriber](https://github.com/khdobromir/transcriber).
Final measured identities and CI run links will be retained beside the immutable
12-asset release in `release-validation.json`; earlier candidate evidence below
is historical and does not validate final-SHA binaries.

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
package. At that stage native MSVC, Qt 6.8.3 package builds and final-SHA remote
CI were pending; the later Linux candidate checks are recorded below.

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

### Linux runtime correction and candidate verification

The Linux correction is recorded in local commits `27e31c6`, `dc505ab` and
`3616a0e`. The last commit is the exact clean source of the reviewed AppImage;
the subsequent acceptance-document commit does not relabel its bytes or SHA.
The implementation restores all eleven AppRun variables for external desktop
processes, preserving absent/empty/custom values and the original snapshot on
re-entry. Recovery stops at the environment on entry to AppRun, not an earlier
AppImage-runtime state. GUI/CLI keep their bundle environment.

A separate asynchronous process monitors xdg-open. The private monitor can be
stopped and reaped without killing the external application; its explicit wait
after killing a monitor is limited to one second. Active duplicate targets are coalesced. Late errors
are scoped by task/selection generation, directory and request; distinct error
properties survive successful preview reads and do not change recognition
status. Native nonportable/Windows opening continues through QDesktopServices.

Portable Fontconfig initialization runs before QGuiApplication in the actual
application and in the separately configured real-GUI harness. It selects the
private config without loading incompatible host templates. Font directories
and the XDG cache are preserved; automatic rescanning is disabled. Linux GUI
builds now require libfontconfig-dev; CLI-only builds remain independent of it.
The pinned builder explicitly refreshes libsystemd0/libudev1/liblzma5, whose base
image versions no longer all had matching sources in current Ubuntu indexes.
No source-retention check or glibc gate was weakened.

Fresh native Qt 6.12 verification passed all 30 CTest suites with local sockets
permitted for the HTTP controller fixtures. The isolated Ubuntu
24.04/Qt 6.8.3 clean-source build passed all 30 together with warnings as errors,
QML lint and all 184 English translations. ASan/UBSan passed the desktop,
portable-font, GUI QML and GUI/CLI integration tests; the final monitor cleanup
and ETA adjustment also passed focused sanitizer reruns. Leak detection was
disabled. New regressions cover all environment states/re-entry, launch failure,
nonzero/crashed handlers, long-running handlers, repeat clicks, shutdown,
selection/task changes and error persistence during preview completion. The
monitor cleanup regression failed against the old code, then passed with the
fix. Python package helper tests passed 20/20; AppRun is covered by shellcheck.

Full clang-tidy 18 inspection exposed one unchecked-optional diagnostic in the
previous ETA code. Computing the same smoothed value in a local double removes
that diagnostic without changing the formula or suppressing checks. Targeted
clang-tidy 18 reruns accepted all subsequent changed C++ files. Full Cppcheck
accepted all 46 compilation entries. Native clang-tidy and changed-line
clang-format 18 checks also passed. The committed source archive was checked
against all 359 Git blobs/types/executable flags and rebuilt as a strict Clang
Release CLI, reporting transcribe 0.5.0 without acquiring Fontconfig.

The exact AppImage SHA-256 is
`381e1ec2cdc06989528333cee7af229a0db12d45218f73bc80ddfebe819f8864`.
It was built with explicit `--release --real-smoke`; required glibc stays within
the automatic 2.39 baseline. The final image passed actual AppRun/main startup,
bundled-tool/Unicode-path checks and the real GUI-to-bundled-CLI recognition of
the pinned public Russian fixture with small-q5_1, including preview and
TXT/SRT/VTT. Healthy startup logs contain no Fontconfig warning/error. Reports
confirm bundled Fontconfig 2.15.0, its loaded-library SHA matching the manifest,
private config and Cyrillic glyphs. A damaged-config copy fails at actual AppRun
startup with complete output retained. The isolated initializer probe also
rejects skipped initialization, missing config and malformed config.

The same unchanged final image passed offline clean Ubuntu 24.04 and Debian 13
startup, ASR and damaged-config checks, with no installed Qt/Python/FFmpeg/tools.
Docker socket access was unavailable, so the pinned OCI roots were run using
Bubblewrap user/network namespaces and the unchanged smoke_linux_clean.sh.
This records equivalent clean payload checks, not a successful Docker build or
execution of the Docker orchestration in check_linux_candidate.py. OCI digests,
commands, full logs and public input hashes are retained for reproduction.

Native Wayland verification on the current Omarchy/Hyprland host used the same
image bytes. Real GUI buttons opened a Unicode result directory in Nautilus and
TXT in Alacritty/Neovim with a private test MIME association. External processes
had no snapshot variables and loaded host GLib/Fontconfig; the GUI environment
remained unchanged. Repeated requests produced one window per target. Closing
Transcribe exited normally and left both desktop applications alive; only the
test windows were subsequently closed. This separately verifies desktop opening;
the offscreen checks alone do not establish it.

Two host observations are retained without claiming fixes outside this scope:
generic xdg-open launches the host's terminal-only nvim.desktop without a new
terminal, so the graphical TXT check used a private association rather than
changing user MIME settings. Qt 6.8.3 also emitted its cached-DPI diagnostic on
native window updates; the windows continued working. No Fontconfig warning was
hidden or accepted as a healthy result.

The current review materials are in `dist/releases/v0.5.0-linux-review/`, including
the exact candidate record, matching source/build-input archives, manifests,
full run logs, font diagnostics, public speech/model inputs and native screenshots.
The earlier v0.5.0-local-review directory still describes its older source and
native build; those old bytes were not renamed as the new portable candidate.
Required evidence is copied out of .cache/linux-package-container/run-* before
temporary build environments are removed. The user .vscode directory is preserved and now ignored. The task review bundle
was removed as authorized on 2026-11-04. Windows acceptance above remains the user's report;
no Windows package hash or source SHA is invented for these Linux checks.

Under the 2026-11-04 authorization, use the release procedure to obtain final-SHA CI,
build the final Windows ZIP/Linux AppImage and matching source/build-input/notice
materials, and record actual hashes and skipped checks. Do not reuse or rename
beta binaries as stable. The canonical 12-asset contract remains defined by
`scripts/release_assets.py`; the locally prepared source archive alone does not
satisfy it. Create an annotated `v0.5.0` tag on verified master and publish as stable latest.
Pushes to master now enable real smoke in CI (installed Linux and Windows ZIP),
Linux portable (including offline Ubuntu/Debian) and Real Whisper smoke. Manual
dispatch remains available and the weekly Real Whisper schedule is unchanged.
Do not manually redispatch the automatic final-master runs.

## Final CI corrections

Final PR CI found a deprecated Qt exception assertion; the desktop regression now
uses `QVERIFY_THROWS_EXCEPTION` with the same exception/operation. Clang with
warnings as errors verifies the actual test translation unit.

The portable font fixture now includes DejaVu Sans and sets `QT_QPA_FONTDIR`, as
the actual AppRun does; official Qt offscreen builds must not depend on host font
discovery. GUI CI explicitly installs that test prerequisite.

LeakSanitizer reproduced a Fontconfig 2.15 allocation leak in the XML relative
prefix handler. The retained exact upstream source shows that the resolved
filename is not freed on success. Removing that XML prefix eliminates the leak;
startup registers the relocated bundle font directory using `FcConfigAppFontAddDir`
instead. Leak detection remains enabled; no suppression was added. Final CI and
package checks cover this correction under the final release identity.

The font diagnostic checks each Cyrillic letter through Qt text shaping, including
font fallback. Checking only the default QRawFont incorrectly rejected hosts whose
Latin primary font delegates Cyrillic to another installed or bundled font. A local
Bitstream Vera primary-font reproducer fails the old check; the shaping check
accepts working fallback and rejects the same font with fallback disabled.

Windows Debug passed the large transcript reading regression, then reached the
30-second aggregate `gui_qml` limit during the following responsive-layout test.
The unchanged transcript regression now runs as `gui_transcript_view` in a separate
process with its own 60-second budget. The existing general QML suite retains its
30-second limit and all layout/theme/keyboard/lifecycle assertions. Both tests
use the same native Windows platform/theme and deployed runtime.
