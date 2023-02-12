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

#include "Time.h"

#include <iostream>

namespace WinTime
{

  void PTime::print() const
  {
      std::cerr << "Creation time " << (t_create) << "\n";
      std::cerr << "    Exit time " << (t_exit) << "\n";
      std::cerr << "    Wall time: " << toTimeDiffString(t_wall) << "\n";
      std::cerr << "    User time: " << toTimeDiffString(t_user) << "\n";
      std::cerr << "  Kernel time: " << toTimeDiffString(t_kernel) << "\n";
  }

  std::string PTime::print(const char separator) const
  {
    std::stringstream where;
    where << t_create << separator
    << t_exit << separator
      << toTimeDiffString(t_wall) << separator
      << toTimeDiffString(t_user) << separator
      << toTimeDiffString(t_kernel);
    return where.str();
  }

  std::string PTime::printHeader(const char separator)
  {
    std::stringstream where;
    where << "creation_time" << separator
      << "exit_time" << separator
      << "wall_time" << separator
      << "user_time" << separator
      << "kernel_time";
    return where.str();
  }


  std::string toDateString(const SYSTEMTIME& time)
  {
    char buffer[100];
    int written_bytes = std::snprintf(buffer, sizeof(buffer), "%i/%.2i/%.2i %.2i:%.2i:%.2i.%.3i", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
    assert(written_bytes < sizeof(buffer) - 1);
    return std::string(buffer);
  }

  uint64_t toInt64(FILETIME t)
  {
    static_assert(std::is_same_v<decltype(ULARGE_INTEGER::QuadPart), uint64_t>);
    ULARGE_INTEGER temp;
    temp.HighPart = t.dwHighDateTime;
    temp.LowPart = t.dwLowDateTime;
    return temp.QuadPart;
  }

  double toSeconds(FILETIME t)
  {
    return toInt64(t) / (1e7); // QuardPart is in 100-nanosecond intervals,i.e. 1e9 / 1e2 of a second
  }

  double toSeconds(FILETIME from, FILETIME to)
  {
    auto diff = toInt64(to) - toInt64(from);
    return (diff) / (1e7); // QuardPart is in 100-nanosecond intervals,i.e. 1e9 / 1e2 of a second
  }

  /// convert seconds to higher units (minute, hour, days)
  std::string toTimeDiffString(const double seconds)
  {
    std::string result;
    int64_t t_sec = int64_t(seconds); // trunc is ok
    auto t_days = (t_sec) / (3600 * 24);
    t_sec -= t_days * (3600 * 24);
    auto t_hour = (t_sec) / 3600;
    t_sec -= t_hour * 3600;
    auto t_min = (t_sec) / 60;
    t_sec -= t_min * 60;
    int t_msec = (int)((seconds - trunc(seconds)) * 1000);
    char buffer[100];
    int written_bytes = std::snprintf(buffer, sizeof(buffer), " %Ii days, %.2Ii:%.2Ii:%.2Ii.%.3i (%.2f seconds)", t_days, t_hour, t_min, t_sec, t_msec, seconds);
    assert(written_bytes < sizeof(buffer) - 1);
    return std::string(buffer);
  }

  PTime getProcessTime(HANDLE hProcess)
  {
    FILETIME lpCreationTime{},
      lpExitTime{},
      lpKernelTime{},
      lpUserTime{};

    if (0 == GetProcessTimes(
      hProcess,
      &lpCreationTime,
      &lpExitTime,
      &lpKernelTime,
      &lpUserTime))
    {
      std::cerr << "Could not query Process timings\n";
      return PTime{};
    }
    SYSTEMTIME t_create, t_exit;
    FileTimeToSystemTime(&lpCreationTime, &t_create);
    FileTimeToSystemTime(&lpExitTime, &t_exit);
    return PTime{ toDateString(t_create),
      toDateString(t_exit),
      toSeconds(lpUserTime),
      toSeconds(lpKernelTime),
      toSeconds(lpCreationTime, lpExitTime) };
  }

} // namespace
