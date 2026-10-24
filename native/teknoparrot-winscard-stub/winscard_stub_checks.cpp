#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winscard.h>
#include <cstdio>
#include <cstring>

static unsigned assertions;

static void Check(bool condition, const char* reason)
{
    ++assertions;
    if (!condition) {
        std::fprintf(stderr, "FAILED: %s\n", reason);
        ExitProcess(1);
    }
}

int main(int argc, char** argv)
{
    Check(argc == 2, "explicit built DLL path required");
    HMODULE library = LoadLibraryExA(argv[1], nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    Check(library != nullptr, "load the supplied x86 payload");
    const char* exports[] = {
        "SCardConnectA", "SCardDisconnect", "SCardEstablishContext", "SCardFreeMemory",
        "SCardGetAttrib", "SCardListReadersA", "SCardReleaseContext", "SCardStatusA",
        "SCardTransmit", "g_rgSCardT0Pci", "g_rgSCardT1Pci"
    };
    for (const char* name : exports)
        Check(GetProcAddress(library, name) != nullptr, name);

    // The actual Windows SDK declaration determines the x86 calling convention.
    decltype(&SCardGetAttrib) getAttrib = nullptr;
    FARPROC address = GetProcAddress(library, "SCardGetAttrib");
    static_assert(sizeof(getAttrib) == sizeof(address), "function pointer ABI");
    std::memcpy(&getAttrib, &address, sizeof(address));
    struct LengthGuard { DWORD before; DWORD value; DWORD after; } length = {
        0x12345678, 16, 0x87654321
    };
    BYTE buffer[16];
    std::memset(buffer, 0xa5, sizeof(buffer));
    SetLastError(0x34567890);
    Check(getAttrib(0x12345678, SCARD_ATTR_ATR_STRING, buffer, &length.value) ==
        SCARD_E_INVALID_HANDLE, "invalid handle result");
    Check(length.value == 0 && length.before == 0x12345678 && length.after == 0x87654321,
        "only the optional DWORD length is written");
    for (BYTE value : buffer)
        Check(value == 0xa5, "attribute buffer untouched");
    Check(GetLastError() == 0x34567890, "last-error value preserved");
    Check(getAttrib(0, 0, nullptr, nullptr) == SCARD_E_INVALID_HANDLE,
        "null optional outputs accepted");
    BYTE* allocated = reinterpret_cast<BYTE*>(0x12345678);
    length.value = SCARD_AUTOALLOCATE;
    Check(getAttrib(1, SCARD_ATTR_ATR_STRING, reinterpret_cast<BYTE*>(&allocated),
        &length.value) == SCARD_E_INVALID_HANDLE, "autoallocate request fails safely");
    Check(allocated == reinterpret_cast<BYTE*>(0x12345678) && length.value == 0,
        "no allocated pointer or card data is fabricated");
    for (unsigned call = 0; call < 10000; ++call)
        Check(getAttrib(call, 0xffffffff, nullptr, nullptr) == SCARD_E_INVALID_HANDLE,
            "repeated stdcall call preserves the caller stack");
    Check(FreeLibrary(library) != FALSE, "unload payload");
    std::printf("WinSCard actual-DLL ABI checks passed: %u assertions\n", assertions);
    return 0;
}
