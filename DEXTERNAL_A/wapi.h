#pragma once
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <Psapi.h>

#define ADDR_MAX 30

int largestram = 0;
int largestPID = 0;

HANDLE dHandle = NULL;

void inject(BYTE vk, DWORD flags)
{
    keybd_event(vk, 0, flags, 0);
}

void AOBSCAN(HANDLE hProcess, const char* pattern, size_t patternSize, uintptr_t* addrarr)
{
    MEMORY_BASIC_INFORMATION memInfo;
    uintptr_t scanAddress = 0;
    int foundcounter = 0;


    while (VirtualQueryEx(hProcess, (LPVOID)scanAddress, &memInfo, sizeof(memInfo)) != 0)
    {
        if (memInfo.Protect != PAGE_NOACCESS && memInfo.State == MEM_COMMIT)
        {
            SIZE_T dummy;

            char* buffer = new char[memInfo.RegionSize + 1];

            if (ReadProcessMemory(dHandle, memInfo.BaseAddress, buffer, memInfo.RegionSize - 1, &dummy))
            {
                buffer[memInfo.RegionSize] = '\0';
                for (size_t i = 0; i < memInfo.RegionSize; i++)
                {
                    for (size_t u = 0; u < patternSize; u++)
                    {
                        if (buffer[u + i] != pattern[u])
                        {
                            break;
                        }

                        if (u == patternSize - 1)
                        {
                            //std::cout << "address: 0x" << std::hex << (uintptr_t)memInfo.BaseAddress + i << std::dec << '\n';

                            addrarr[foundcounter] = (uintptr_t)memInfo.BaseAddress + i;
                            foundcounter++;
                        }
                    }
                }

                //std::cout << "region size: 0x" << std::hex << (uintptr_t)memInfo.RegionSize << std::dec<< '\n';
            }

            buffer[memInfo.RegionSize] = '\0';

            delete[] buffer;
        }
        scanAddress += memInfo.RegionSize;
    }

    return;
}

std::string ReadClipboard()
{
    // Try opening the clipboard
    if (!OpenClipboard(nullptr)) { return "err1"; }

    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) { return "err2"; }

    // Lock the handle to get the actual text pointer
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) { return "err3"; }

    // Save text in a string class instance
    std::string text(pszText);

    // Release the lock
    GlobalUnlock(hData);

    // Release the clipboard
    CloseClipboard();

    return text;
}

void WriteClipboard(const char* text)
{
    int size = ::MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (size < 0) {
        return;
    }

    if (::OpenClipboard(NULL))
    {
        ::EmptyClipboard();
        HGLOBAL hGlobal = ::GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, (size + 1) * sizeof(WCHAR));
        if (hGlobal != NULL)
        {
            LPWSTR lpszData = (LPWSTR)::GlobalLock(hGlobal);
            if (lpszData != nullptr)
            {
                ::MultiByteToWideChar(CP_UTF8, 0, text, -1, lpszData, size);
                ::GlobalUnlock(hGlobal);
                ::SetClipboardData(CF_UNICODETEXT, hGlobal);
            }
        }
        ::CloseClipboard();
    }
}

void FindDiscord()
{
    // grab the correct discord handle (the one that uses the most RAM)
    unsigned long long ramarr[6] = {}; // only 6 discord.exe s

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        int processcounter = 0;
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_stricmp(entry.szExeFile, "Discord.exe") == 0)
            {
                HANDLE currenthProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

                PROCESS_MEMORY_COUNTERS buffer;
                GetProcessMemoryInfo(currenthProcess, &buffer, sizeof(PROCESS_MEMORY_COUNTERS));

                if (buffer.WorkingSetSize > largestram)
                {
                    largestram = buffer.WorkingSetSize;
                    largestPID = entry.th32ProcessID;
                }

                CloseHandle(currenthProcess);
                processcounter++;
                if (processcounter == 6)
                {
                    break;
                }
            }
        }
    }

    std::cout << "largest ram pid: " << largestPID << '\n';
    CloseHandle(snapshot);

    dHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, largestPID);
}

void ReplaceMsg(std::string messagetochange, std::string changeto)
{
    uintptr_t* addrarr = new uintptr_t[ADDR_MAX];

    for (size_t i = 0; i < ADDR_MAX; i++)
    {
        addrarr[i] = 0;
    }

    const char* pattern = messagetochange.c_str();
    size_t patternsize = messagetochange.length();
    AOBSCAN(dHandle, pattern, patternsize, addrarr);

    std::string writestr = changeto;
    int spacesneeded = patternsize - writestr.length();

    for (size_t i = 0; i < spacesneeded; i++)
    {
        writestr += " ";
    }

    for (size_t i = 0; i < ADDR_MAX; i++)
    {
        std::cout << "addr[" << i << "]: 0x" << std::hex << addrarr[i] << std::dec << '\n';
        if (addrarr[i] != 0)
        {
            SIZE_T dummy;
            WriteProcessMemory(dHandle, (LPVOID)addrarr[i], writestr.c_str(), writestr.length(), &dummy);
        }
    }

    delete[] addrarr;
}
