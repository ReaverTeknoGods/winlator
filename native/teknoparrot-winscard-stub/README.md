# TeknoParrot WinSCard compatibility stub

This small x86 DLL supplies the WinSCard imports used by legacy Taito `Dic32.dll`
files when the Wine build has no PC/SC backend. It intentionally reports that the
smart-card service is unavailable; it does not emulate a card or contain Microsoft
code. The Android `taito-legacy-scard` launch preset installs the DLL into the
managed Wine prefix and forces Wine to load it as a native compatibility library.

The exact Music GunGun 2 prepared launch also installs it while preserving the
title's media preset. Each eligible launch verifies the copied asset, so a prior
game is not required to populate the prefix. `SCardGetAttrib` uses the x86 stdcall
ABI and reports an invalid card handle without writing to the caller's buffer or
allocating memory; no card service or successful card read is simulated.

After rebuilding the native DLL, copy that exact payload from
`native/teknoparrot-bridge/bin/winscard-x86.dll` to the companion's tracked
`app/app/src/main/assets/teknoparrot/winscard-x86.dll`. The Gradle gate rejects
differing built/tracked copies instead of silently packaging an older asset.
