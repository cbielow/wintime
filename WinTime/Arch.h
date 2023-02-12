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
#include <string>

namespace WinTime
{

  enum class Arch
  {
    x86,
    x64,
    OTHER, ///< could be MS-DOS, POSIX, 16bit Windows, etc...
    NOT_EXECUTABLE
  };

  /// Get architecture (32 or 64 bit) of executable
  /// Return Arch::NOT_EXECUTABLE on error
  Arch getArch(LPCWSTR path_to_exe);

  constexpr bool hostIs64Bit()
  {
    return sizeof(void*) == 8;
  }

  enum class ArchMatched
  {
    SAME,  ///< all 32bit or all 64bit, i.e. compatible
    MIXED, ///< one is 32bit, the other is 64bit (or vice versa)
    TARGET_UNKNOWN  ///< we surely know the host target (that's us; but target might be Arch::OTHER etc)
  };

  std::wstring getArchMatchedExplanation(const ArchMatched what, LPCWSTR path_to_target_exe);;

  /// Does the architecture of @p target_exe match the architecture of the current process?
  ArchMatched checkMatchingArch(LPCWSTR target_exe);

} // namespace 
