# TeknoParrot Winlator package

This fork publishes Winlator as a separate Android runtime package for
TeknoParrotUI. It is not embedded in the TeknoParrotUI repository or APK.

## Package contract

- Package/workflow repository: `ReaverTeknoGods/winlator`
- Application-source repository: `ReaverTeknoGods/winlator-app`
- Android package identity: `com.teknoparrot.winlator`
- Rolling release tag: `winlator`
- Asset: `TeknoParrotWinlator-<four-part-version>-android-arm64.apk`
- GitHub release name: the same four-part version (used by TPUI's update check)
- Companion metadata:
  `TeknoParrotWinlator-<four-part-version>-android-arm64.apk.json`
- Digest:
  `TeknoParrotWinlator-<four-part-version>-android-arm64.apk.sha256`

The APK is intentionally thin. TeknoParrot, OpenParrot, ElfLoader2, CXBXR, and
PCSX2X6 are updater-owned packages and must not appear in this repository or
APK. Winlator has no runtime-embedding build mode. It receives those archives
from the signed TeknoParrotUI client and installs them transactionally in its
private storage.

The pipe/shared-page/bootstrap helpers and WinSCard compatibility shim are part
of this Winlator integration. The TeknoParrot build server builds them from the
source below `native/`; generated Windows binaries remain ignored.

The build server also opens every tracked compressed payload, rejects unsafe
archive paths, and fails if an archive contains private emulator/core files,
PDB/debug payloads, or protected-core implementation strings.

## Signing

Production publication requires the same signing identity used by the
TeknoParrotUI Android package:

- `ANDROID_SIGNING_KEYSTORE_BASE64`
- `ANDROID_SIGNING_STORE_PASSWORD`
- `ANDROID_SIGNING_KEY_ALIAS`
- `ANDROID_SIGNING_KEY_PASSWORD`

BuilderBill requires production signing and publishes the verified APK plus
size and SHA256 metadata. Publication replaces the previous Winlator APK and
sidecars on the rolling tag so the updater can never select a stale matching
asset. Its local pipeline performs the same build and validation while keeping
Discord and GitHub publication disabled.

## Submodule requirement

The TeknoParrot application changes live in the `app` submodule, sourced from
`ReaverTeknoGods/winlator-app`. Before publication, the application-source
repository must contain a reviewed commit and the top-level Git link must pin
that exact commit. The build server fails closed when the standalone checkout,
pinned submodule, and top-level Git link disagree, or when the pinned source
does not contain the TeknoParrot bridge source and package identity.
