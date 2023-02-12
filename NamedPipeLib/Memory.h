/*
 * Copyright (c) 2023-2023 Chris Bielow <chris[dot]bielow[at]fu-berlin[dot].de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <array>
#include <iomanip>
#include <iostream>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <sstream>

 // To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
 // and compile with -DPSAPI_VERSION=1
#define PSAPI_VERSION 1
#include <psapi.h>    // for PROCESS_MEMORY_COUNTERS
#pragma comment (lib, "Psapi.lib")

namespace WinTime
{

  /// convert bytes to a human readable unit (TiB, GiB, MiB, KiB)
  inline std::string toHumanReadable(uint64_t bytes)
  {
    std::array units{ "byte", "KiB", "MiB", "GiB", "TiB", "PiB" };

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

  /// A serializable wrapper around a 'PROCESS_MEMORY_COUNTERS' struct
  struct ClientProcessMemoryCounter
  {
    explicit ClientProcessMemoryCounter(PROCESS_MEMORY_COUNTERS data)
      : data_(data)
    {
    }

    explicit ClientProcessMemoryCounter(char* data)
      : data_(*reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(data))
    {
    }

    const char* asCharP() const
    {
      return reinterpret_cast<const char*>(&data_);
    }

    void print() const
    { // only report data which does not depend on the time of measurement (which was when unloading the Dll). Only report maximia (peaks).
      std::cerr << "PageFaultCount: " << (data_.PageFaultCount) << '\n';
      std::cerr << "PeakWorkingSetSize: " << toHumanReadable(data_.PeakWorkingSetSize) << '\n';
      //std::cerr << "WorkingSetSize: " << toHumanReadable(data_.WorkingSetSize) << '\n';
      std::cerr << "QuotaPeakPagedPoolUsage: " << toHumanReadable(data_.QuotaPeakPagedPoolUsage) << '\n';
      //std::cerr << "QuotaPagedPoolUsage: " << toHumanReadable(data_.QuotaPagedPoolUsage) << '\n';
      std::cerr << "QuotaPeakNonPagedPoolUsage: " << toHumanReadable(data_.QuotaPeakNonPagedPoolUsage) << '\n';
      //std::cerr << "QuotaNonPagedPoolUsage: " << toHumanReadable(data_.QuotaNonPagedPoolUsage) << '\n';
      //std::cerr << "PagefileUsage: " << toHumanReadable(data_.PagefileUsage) << '\n';
      std::cerr << "PeakPagefileUsage: " << toHumanReadable(data_.PeakPagefileUsage) << '\n';
    }

    std::string print(const char separator) const
    {
      std::stringstream where;
      where << data_.PageFaultCount << separator
        <<                (data_.PeakWorkingSetSize) << separator 
        << toHumanReadable(data_.PeakWorkingSetSize) << separator
        //<< toHumanReadable(data_.WorkingSetSize) << separator
        << toHumanReadable(data_.QuotaPeakPagedPoolUsage) << separator
        //<< toHumanReadable(data_.QuotaPagedPoolUsage) << separator
        << toHumanReadable(data_.QuotaPeakNonPagedPoolUsage) << separator
        //<< toHumanReadable(data_.QuotaNonPagedPoolUsage) << separator
        //<< toHumanReadable(data_.PagefileUsage) << separator
        << toHumanReadable(data_.PeakPagefileUsage);
      return where.str();
    }

    static std::string printHeader(const char separator)
    { // only report data which does not depend on the time of measurement (which was when unloading the Dll). Only report maximia (peaks).
      std::stringstream where;
      where << "PageFaultCount" << separator
        << "PeakWorkingSetSize (bytes)" << separator
        << "PeakWorkingSetSize" << separator
        //<< "WorkingSetSize" << separator
        << "QuotaPeakPagedPoolUsage" << separator
        //<< "QuotaPagedPoolUsage" << separator
        << "QuotaPeakNonPagedPoolUsage" << separator
        //<< "QuotaNonPagedPoolUsage" << separator
        //<< "PagefileUsage" << separator
        << "PeakPagefileUsage";
      return where.str();
    }

    PROCESS_MEMORY_COUNTERS data_;
  };

} // namespace