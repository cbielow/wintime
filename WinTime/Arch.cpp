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
#define _CRT_SECURE_NO_WARNINGS 1

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h> // for HANDLE; <windef.h> alone is not sufficient
#undef max
#undef min

#include <stdexcept>


#include "Arch.h"
#include "Process.h"

using namespace std;

namespace WinTime
{
  Arch getArch(const std::string& path_to_exe)
  {
    DWORD result;
    if (!GetBinaryTypeW(&widen(path_to_exe)[0], &result))
    { // not an executable
      return Arch::NOT_EXECUTABLE;
    }

    switch (result)
    {
    break; case SCS_32BIT_BINARY:
      return Arch::x86;
    break; case SCS_64BIT_BINARY:
      return Arch::x64;
    break; default:
      // we cannot handle MSDOS, POSIX and other 16bit apps
      return Arch::OTHER;
    }
  }

  std::string getArchMatchedExplanation(const ArchMatched what, LPCSTR path_to_target_exe)
  {
    switch (what)
    {
    break; case ArchMatched::SAME:
      return std::string("Architectures match") + (sizeof(void*) == 8 ? " (64bit)" : " (32bit)");
    break; case ArchMatched::MIXED:
      if (hostIs64Bit())
        return std::string("WinTime is 64 bit, but target (") + path_to_target_exe + ") is 32 bit. Please use 32 bit version of WinTime.";
      else
        return std::string("WinTime is 32 bit, but target (") + path_to_target_exe + ") is 64 bit. Please use 64 bit version of WinTime.";
    break; case ArchMatched::TARGET_UNKNOWN:
      return std::string("Target Architecture (of ") + path_to_target_exe + ") is neither 32bit nor 64bit Windows, but something else.";
    default:
      throw std::logic_error("Missed a case? Please report a bug!");
    };

  }

  ArchMatched checkMatchingArch(const std::string& target_exe)
  {
    const auto target_arch = getArch(target_exe);
    constexpr const Arch this_arch = hostIs64Bit() ? Arch::x64 : Arch::x86;
    if (target_arch == this_arch) return ArchMatched::SAME;
    if (target_arch == Arch::OTHER || target_arch == Arch::NOT_EXECUTABLE) return ArchMatched::TARGET_UNKNOWN;
    return ArchMatched::MIXED;
  };

} // namespace
