CC64 ?= x86_64-w64-mingw32-gcc
CC32 ?= i686-w64-mingw32-gcc
OUT_DIR ?= bin
CFLAGS ?= -O2 -Wall -Wextra -static

WINSCARD_SOURCE = ../teknoparrot-winscard-stub/winscard_stub.c
WINSCARD_DEF = ../teknoparrot-winscard-stub/winscard_stub.def

all: \
	$(OUT_DIR)/bridgeguest64.exe \
	$(OUT_DIR)/bridgeguest32.exe \
	$(OUT_DIR)/pipehelper.exe \
	$(OUT_DIR)/pipehelper32.exe \
	$(OUT_DIR)/windows-path-bootstrap.exe \
	$(OUT_DIR)/winscard-x86.dll \
	$(OUT_DIR)/android-winlator-service-bridge-diagnostics.bat

$(OUT_DIR):
	mkdir -p $@

$(OUT_DIR)/bridgeguest64.exe: winlator_bridge_guest.c | $(OUT_DIR)
	$(CC64) $(CFLAGS) -o $@ $<

$(OUT_DIR)/bridgeguest32.exe: winlator_bridge_guest.c | $(OUT_DIR)
	$(CC32) $(CFLAGS) -o $@ $<

$(OUT_DIR)/pipehelper.exe: pipehelper.c | $(OUT_DIR)
	$(CC64) $(CFLAGS) -o $@ $< -lws2_32

$(OUT_DIR)/pipehelper32.exe: pipehelper.c | $(OUT_DIR)
	$(CC32) $(CFLAGS) -o $@ $< -lws2_32

$(OUT_DIR)/windows-path-bootstrap.exe: windows_path_bootstrap.c | $(OUT_DIR)
	$(CC32) $(CFLAGS) -municode -o $@ $< -luser32

$(OUT_DIR)/winscard-x86.dll: $(WINSCARD_SOURCE) | $(OUT_DIR)
	# GCC emits the dllexport directives from the source itself.  --kill-at
	# publishes the WinSCard-compatible undecorated stdcall names; the separate
	# .def remains for the MSVC build in build.ps1.
	$(CC32) -O2 -Wall -Wextra -shared -static -Wl,--kill-at \
		-o $@ $(WINSCARD_SOURCE)

$(OUT_DIR)/android-winlator-service-bridge-diagnostics.bat: \
		android-winlator-service-bridge-diagnostics.bat | $(OUT_DIR)
	cp $< $@

clean:
	rm -rf $(OUT_DIR)

.PHONY: all clean
