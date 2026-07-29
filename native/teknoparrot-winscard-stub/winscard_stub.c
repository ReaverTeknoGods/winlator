#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef ULONG_PTR SCARDCONTEXT;
typedef ULONG_PTR SCARDHANDLE;

typedef struct _TP_SCARD_IO_REQUEST {
    DWORD protocol;
    DWORD length;
} TP_SCARD_IO_REQUEST;

typedef TP_SCARD_IO_REQUEST *PTP_SCARD_IO_REQUEST;
typedef const TP_SCARD_IO_REQUEST *PCTP_SCARD_IO_REQUEST;

#define TP_SCARD_PROTOCOL_T0 0x00000001u
#define TP_SCARD_PROTOCOL_T1 0x00000002u
#define TP_SCARD_E_NO_SERVICE ((LONG)0x8010001Du)
#define TP_SCARD_E_INVALID_HANDLE ((LONG)0x80100003u)
#define TP_SCARD_S_SUCCESS ((LONG)0x00000000u)

__declspec(dllexport) TP_SCARD_IO_REQUEST g_rgSCardT0Pci = {
    TP_SCARD_PROTOCOL_T0, sizeof(TP_SCARD_IO_REQUEST)
};

__declspec(dllexport) TP_SCARD_IO_REQUEST g_rgSCardT1Pci = {
    TP_SCARD_PROTOCOL_T1, sizeof(TP_SCARD_IO_REQUEST)
};

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    (void)instance;
    (void)reason;
    (void)reserved;
    return TRUE;
}

__declspec(dllexport) LONG WINAPI SCardEstablishContext(
    DWORD scope, LPCVOID reserved1, LPCVOID reserved2, SCARDCONTEXT *context)
{
    (void)scope;
    (void)reserved1;
    (void)reserved2;
    if (context != NULL)
        *context = 0;
    return TP_SCARD_E_NO_SERVICE;
}

__declspec(dllexport) LONG WINAPI SCardReleaseContext(SCARDCONTEXT context)
{
    (void)context;
    return TP_SCARD_S_SUCCESS;
}

__declspec(dllexport) LONG WINAPI SCardConnectA(
    SCARDCONTEXT context, LPCSTR reader, DWORD shareMode, DWORD preferredProtocols,
    SCARDHANDLE *card, DWORD *activeProtocol)
{
    (void)context;
    (void)reader;
    (void)shareMode;
    (void)preferredProtocols;
    if (card != NULL)
        *card = 0;
    if (activeProtocol != NULL)
        *activeProtocol = 0;
    return TP_SCARD_E_NO_SERVICE;
}

__declspec(dllexport) LONG WINAPI SCardDisconnect(SCARDHANDLE card, DWORD disposition)
{
    (void)card;
    (void)disposition;
    return TP_SCARD_S_SUCCESS;
}

__declspec(dllexport) LONG WINAPI SCardListReadersA(
    SCARDCONTEXT context, LPCSTR groups, LPSTR readers, DWORD *readerCount)
{
    (void)context;
    (void)groups;
    (void)readers;
    if (readerCount != NULL)
        *readerCount = 0;
    return TP_SCARD_E_NO_SERVICE;
}

__declspec(dllexport) LONG WINAPI SCardStatusA(
    SCARDHANDLE card, LPSTR readerName, DWORD *readerNameLength, DWORD *state,
    DWORD *protocol, BYTE *attribute, DWORD *attributeLength)
{
    (void)card;
    (void)readerName;
    (void)readerNameLength;
    (void)state;
    (void)protocol;
    (void)attribute;
    (void)attributeLength;
    return TP_SCARD_E_INVALID_HANDLE;
}

__declspec(dllexport) LONG WINAPI SCardTransmit(
    SCARDHANDLE card, PCTP_SCARD_IO_REQUEST sendPci, const BYTE *sendBuffer,
    DWORD sendLength, PTP_SCARD_IO_REQUEST receivePci, BYTE *receiveBuffer,
    DWORD *receiveLength)
{
    (void)card;
    (void)sendPci;
    (void)sendBuffer;
    (void)sendLength;
    (void)receivePci;
    (void)receiveBuffer;
    (void)receiveLength;
    return TP_SCARD_E_INVALID_HANDLE;
}

__declspec(dllexport) LONG WINAPI SCardFreeMemory(SCARDCONTEXT context, LPCVOID memory)
{
    (void)context;
    (void)memory;
    return TP_SCARD_S_SUCCESS;
}
