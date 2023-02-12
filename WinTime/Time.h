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

#include <cassert>
#include <optional>

#include <sstream>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define UNICODE 1
 // Windows Header Files:
#include <windows.h>


namespace WinTime
{
  std::string toDateString(const SYSTEMTIME& time);

  uint64_t toInt64(FILETIME t);

  double toSeconds(FILETIME t);
  double toSeconds(FILETIME from, FILETIME to);

  /// convert seconds to higher units (minute, hour, days)
  std::string toTimeDiffString(const double seconds);

  /// Process time data aggregator
  struct PTime
  {
    std::string t_create;
    std::string t_exit;
    double t_kernel;
    double t_user;
    double t_wall;

    void print() const;

    std::string print(const char separator) const;

    static std::string printHeader(const char separator);

  };


  PTime getProcessTime(HANDLE hProcess);

} // namespace WinTime
