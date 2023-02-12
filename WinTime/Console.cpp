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
 
#include "Console.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h> // for HANDLE; <windef.h> alone is not sufficient
#undef max
#undef min
#include <iostream>


using namespace std;

namespace WinTime
{

  int Console::readConsoleSize_()
  {
    // avoid calling this function more than once
    static bool been_here = false;
    if (been_here)
    {
      return console_width_;
    }

    been_here = true;

    // determine column width of current console
    try
    {
      console_width_ = -1;
      char* p_env = getenv("COLUMNS");
      if (p_env)
      {
        console_width_ = atoi(p_env);
      }
      else
      {
        std::cerr << "output shaping: COLUMNS env does not exist!" << std::endl;

        HANDLE hOut;
        CONSOLE_SCREEN_BUFFER_INFO SBInfo;
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hOut, &SBInfo);
        console_width_ = SBInfo.dwSize.X;
      }
      --console_width_; // to add the \n at the end of each line without forcing another line break on windows
    }
    catch (...)
    {
    }
    // if console_width_ is still -1 or too small, we do not use command line reshaping
    if (console_width_ < 10)
    {
      std::cerr << "Console width could not be determined or is smaller than 10. Not using output shaping!" << std::endl;
      console_width_ = std::numeric_limits<int>::max();
    }

    return console_width_;
  }

}