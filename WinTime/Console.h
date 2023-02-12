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

#include <limits>

namespace WinTime
{
  /**
   *
   * Determines the width of the console automatically.
   *
   * To manually force a certain width set the environment variable 'COLUMNS' to a desired value.
   *
   */
  class Console
  {
  private:
    /// C'tor (private) -- use Console::getInstance()
    Console();

  public:
    /// Copy C'tor (deleted)
    Console(const Console&) = delete;

    /// Assignment operator (deleted)
    void operator=(Console const&) = delete;

    /// returns the singleton -- the only instanciation of this class
    static const Console& getInstance();

    /// width of the console (or INTMAX on internal error)
    int getConsoleWidth() const
    {
      return console_width_;
    }

  private:
    /// width of console we are currently in (if not determinable, set to INTMAX, i.e. no breaks)
    int console_width_ = std::numeric_limits<int>::max();

    /// read console settings for output shaping
    int readConsoleSize_();
  };

} // namespace
