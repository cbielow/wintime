#include <iostream>
#include <thread>

#include <array>
#include <iomanip>
#include <sstream>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
 // and compile with -DPSAPI_VERSION=1
#define PSAPI_VERSION 1
#include <psapi.h>        // for PROCESS_MEMORY_COUNTERS
#pragma comment (lib, "Psapi.lib")


/// convert bytes to a human readable unit (TiB, GiB, MiB, KiB)
inline std::string toHumanReadable(uint64_t bytes)
{
  std::array<const char*, 6> units{ "byte", "KiB", "MiB", "GiB", "TiB", "PiB" };

  const int divisor = 1024;
  double db = double(bytes);
  for (const auto u : units)
  {
    if (db < divisor)
    {
      std::stringstream ss;
      ss << std::setprecision(4) /* 4 digits overall, i.e. 1000 or 1.456 */ << db << ' ' << u;
      return ss.str();
    }
    db /= divisor;
  }
  // wow ... you made it here... 
  return std::string("Congrats. That's a lot of bytes: ") + std::to_string(bytes);
}

void getMemoryInfo(DWORD processID)
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
    std::cerr << "PageFaultCount: " << (pmc.PageFaultCount) << '\n';
    std::cerr << "PeakWorkingSetSize: " << toHumanReadable(pmc.PeakWorkingSetSize) << '\n';
    std::cerr << "WorkingSetSize: " << toHumanReadable(pmc.WorkingSetSize) << '\n';
    std::cerr << "QuotaPeakPagedPoolUsage: " << toHumanReadable(pmc.QuotaPeakPagedPoolUsage) << '\n';
    std::cerr << "QuotaPagedPoolUsage: " << toHumanReadable(pmc.QuotaPagedPoolUsage) << '\n';
    std::cerr << "QuotaPeakNonPagedPoolUsage: " << toHumanReadable(pmc.QuotaPeakNonPagedPoolUsage) << '\n';
    std::cerr << "QuotaNonPagedPoolUsage: " << toHumanReadable(pmc.QuotaNonPagedPoolUsage) << '\n';
    std::cerr << "PagefileUsage: " << toHumanReadable(pmc.PagefileUsage) << '\n';
    std::cerr << "PeakPagefileUsage: " << toHumanReadable(pmc.PeakPagefileUsage) << '\n';
    std::cerr.flush();
  }

  CloseHandle(hProcess);
}

int main(int argc, char** argv)
{
  std::cerr << "Hello World from an example app!\nArgs given were:";
  for (int i = 0; i < argc; ++i)
  {
    std::cerr << " " << argv[i];
  }
  std::cerr << "\n\n";

  if (argc == 1)
  {
    std::cerr << "Usage " << argv[0] << " <MiB alloc> <ms sleep> <stats>\n"
                 "  MiB alloc: [number] MiB to allocate (this may take some time, especially for larger allocations)\n"
                 "  ms sleep:  [number, optional] Milliseconds to sleep (in addition to the time it takes to allocate -- see above)\n"
                 "  stats:     [any value, optional] Print memory usage using internal functions (useful to compare against the results of an external WinTime call)\n";
  } 
  if (argc >= 2)
  {
    size_t mb = std::abs(atoi(argv[1]));
    std::cerr << "  -- Allocating " << mb << " Mb\n";
    size_t bytes_to_allocate = mb * 1024 * 1024;
    std::string large_data(bytes_to_allocate, 0);
    volatile char* vd = large_data.data();
  }
  if (argc >= 3)
  {
    size_t ms = std::abs(atoi(argv[2]));
    std::cerr << "  -- Sleeping " << ms << " milliseconds\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    
    std::cerr << "  -- wake up!\n";
  }
  if (argc >= 4) 
  {
    getMemoryInfo(GetCurrentProcessId());
  }
  std::cerr << "-- end of ExampleTarget\n\n";
}


