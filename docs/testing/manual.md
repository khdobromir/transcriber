# Optional manual checks

These checks are optional and are not a prerequisite for release publication.
Record any untested scenarios explicitly; automated tests cannot establish them.

Record the exact package filename, SHA-256, manifest source/version, OS build,
CPU/RAM, display scale, model/VAD and chunk settings. Use a separate data/output
directory. Do not publish private media, transcripts, URLs or cookies.

On Windows 11 x64, unpack the complete ZIP into a path containing spaces and
Unicode. Use a machine without the Qt/Visual Studio/Python/media-tool SDKs.
Compare `Get-FileHash -Algorithm SHA256 -LiteralPath '.\Transcribe-….zip'` with
the digest for that asset on GitHub Releases, then run `bin\transcribe-gui.exe`.
On Linux, use the actual AppImage and record distribution/glibc and
Wayland/X11 session. Automated clean-container checks complement the desktop test.

1. Start the GUI; verify model download and import, including VAD.
2. Transcribe a local speech file. Observe preview, completion and readable
   TXT/SRT/VTT; copy text and open the result directory.
3. Cancel while processing, inspect retained partial text, retry, then close
   during an active task. Confirm no owned backend/media processes remain.
4. Restart; verify history and exports. Check keyboard navigation, native dialogs,
   minimum window size, light/dark appearance and 100/150/200% display scales.
5. Record default medium + VAD and explicitly selected turbo/chunks separately.
6. If testing a URL, use only an authorized source. Record URL/cache and optional
   authentication results separately from local-file results.
7. While recognition adds text, scroll up and select/copy a passage. The displayed
   text, selection and reading position must remain stable. Use **Jump to new
   text**, or clear the selection and scroll to the bottom, to resume following.
   Repeat with a transcript larger than 512 KiB: the held passage must remain
   readable while the current preview advances to its latest 512 KiB. Switch to
   another history result and verify that its text replaces the held passage.
8. Check ETA during recognition: numbers change no more often than once per five
   seconds, use rounded minutes (hours for long tasks), and five-second steps
   below one minute. After 30 seconds without progress, the label says it is
   refining the estimate. On recovery it waits for a fresh estimate. Saving,
   cancellation and terminal states hide ETA; the label never declares success.

Mark each scenario passed, failed (with reproduction) or not checked. Runner CI
does not establish native dialogs, clipboard or a clean Windows 11 desktop.

The [original Windows checklist](../archive/windows-11-manual-validation.md)
is retained as historical context. For v0.4.0, the maintainer waived Ubuntu
desktop acceptance, cookies and a 10–15 minute recording; those checks were not
claimed passed. See the [release journal](../releases/v0.4.0/validation.md).
Each release records its actual testing scope and remaining limitations.
