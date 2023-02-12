
#include "NamedPipeClient.h"

// from NamedPipeLib
#include <Defs.h>
#include <Memory.h>

#include <codecvt>
#include <iostream>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>


void sendMemoryInfo(DWORD processID)
{
  HANDLE hProcess;
  PROCESS_MEMORY_COUNTERS pmc;

  // Print information about the memory usage of the process.
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
    PROCESS_VM_READ,
    FALSE, processID);
  if (NULL == hProcess)
    return;

  if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
  {
    WinTime::ClientProcessMemoryCounter pmc_pipe(pmc);
    WinTime::NamedPipeClient::sendData(pmc_pipe.asCharP(), sizeof(pmc_pipe));
  }

  CloseHandle(hProcess);
}

void work()
{
  sendMemoryInfo(GetCurrentProcessId());
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
  switch (ul_reason_for_call)
  {
    break; case DLL_PROCESS_ATTACH:
    //  std::cerr << "attach ProcessInfoDLL\n";
    break; case DLL_THREAD_ATTACH:
    break; case DLL_THREAD_DETACH:
    break; case DLL_PROCESS_DETACH:
      work();
    //  std::cerr << "detach ProcessInfoDLL\n";
  }
  return TRUE;
}


