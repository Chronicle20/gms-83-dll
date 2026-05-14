#include "memedit.h"
#include <vector>

BOOL MemEdit::PatchRetZero(DWORD dwAddress) {
    BYTE bArr[3];
    bArr[0] = x86XOR;
    bArr[1] = x86EAXEAX;
    bArr[2] = x86RET;

    // https://stackoverflow.com/a/13026295/14784253
    DWORD dwOldValue, dwTemp;

    VirtualProtect((LPVOID)dwAddress, sizeof(bArr), PAGE_EXECUTE_READWRITE, &dwOldValue);
    BOOL bSuccess = WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &bArr, sizeof(bArr), nullptr);
    VirtualProtect((LPVOID)dwAddress, sizeof(bArr), dwOldValue, &dwTemp);
    return bSuccess;
}

BOOL MemEdit::PatchJmp(DWORD dwAddress, PVOID pDestination) {
    patch_far_jmp pWrite = {x86JMP, (DWORD)pDestination - (dwAddress + sizeof(DWORD) + sizeof(BYTE))};

    // https://stackoverflow.com/a/13026295/14784253
    DWORD dwOldValue, dwTemp;

    VirtualProtect((LPVOID)dwAddress, sizeof(pWrite), PAGE_EXECUTE_READWRITE, &dwOldValue);
    BOOL bSuccess = WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &pWrite, sizeof(pWrite), nullptr);
    VirtualProtect((LPVOID)dwAddress, sizeof(pWrite), dwOldValue, &dwTemp);
    return bSuccess;
}

BOOL MemEdit::PatchCall(DWORD dwAddress, PVOID pDestination) {
    patch_call pWrite = {x86CALL, (DWORD)pDestination - (dwAddress + sizeof(DWORD) + sizeof(BYTE))};

    // https://stackoverflow.com/a/13026295/14784253
    DWORD dwOldValue, dwTemp;

    VirtualProtect((LPVOID)dwAddress, sizeof(pWrite), PAGE_EXECUTE_READWRITE, &dwOldValue);
    BOOL bSuccess = WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &pWrite, sizeof(pWrite), nullptr);
    VirtualProtect((LPVOID)dwAddress, sizeof(pWrite), dwOldValue, &dwTemp);
    return bSuccess;
}

BOOL MemEdit::PatchNop(DWORD dwAddress, UINT nCount) {
    constexpr UINT kStackThreshold = 64;
    BYTE stackBuf[kStackThreshold];
    std::vector<BYTE> heapBuf;
    BYTE* bArr;

    // SBO: every current caller is <= 7 bytes; vector fallback exists for correctness if a future caller exceeds 64.
    if (nCount <= kStackThreshold) {
        bArr = stackBuf;
    } else {
        heapBuf.assign(nCount, 0);
        bArr = heapBuf.data();
    }

    for (UINT i = 0; i < nCount; ++i)
        bArr[i] = x86NOP;

    // https://stackoverflow.com/a/13026295/14784253
    DWORD dwOldValue, dwTemp;

    VirtualProtect((LPVOID)dwAddress, nCount, PAGE_EXECUTE_READWRITE, &dwOldValue);
    BOOL bSuccess = WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, bArr, nCount, nullptr);
    VirtualProtect((LPVOID)dwAddress, nCount, dwOldValue, &dwTemp);
    return bSuccess;
}

void MemEdit::FillBytes(const DWORD dwOriginAddress, const unsigned char ucValue, const int nCount) {
    memset((void*)dwOriginAddress, ucValue, nCount);
}

void MemEdit::WriteByte(const DWORD dwOriginAddress, const unsigned char ucValue) {
    *(unsigned char*)dwOriginAddress = ucValue;
}

void MemEdit::WriteInt(const DWORD dwOriginAddress, const unsigned int dwValue) {
    *(unsigned int*)dwOriginAddress = dwValue;
}

void MemEdit::CodeCave(void* ptrCodeCave, const DWORD dwOriginAddress, const int nNOPCount) {
    __try {
        if (nNOPCount)
            PatchNop(dwOriginAddress, nNOPCount);
        BYTE jmp = x86JMP;
        WriteValue<BYTE>(dwOriginAddress, &jmp);
        INT ret = (int)(((int)ptrCodeCave - (int)dwOriginAddress) - 5);
        WriteValue<INT>(dwOriginAddress + 1, &ret);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

BOOL MemEdit::WriteBytes(DWORD dwAddress, LPCVOID pData, UINT nCount) {
    // https://stackoverflow.com/a/13026295/14784253
    DWORD dwOldValue, dwTemp;

    VirtualProtect((LPVOID)dwAddress, nCount, PAGE_EXECUTE_READWRITE, &dwOldValue);
    BOOL bSuccess = WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, pData, nCount, nullptr);
    VirtualProtect((LPVOID)dwAddress, nCount, dwOldValue, &dwTemp);
    return bSuccess;
}