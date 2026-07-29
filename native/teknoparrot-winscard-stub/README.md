# TeknoParrot WinSCard compatibility stub

This small x86 DLL supplies the WinSCard imports used by legacy Taito `Dic32.dll`
files when the Wine build has no PC/SC backend. It intentionally reports that the
smart-card service is unavailable; it does not emulate a card or contain Microsoft
code. The Android `taito-legacy-scard` launch preset installs the DLL into the
managed Wine prefix and forces Wine to load it as a native compatibility library.
