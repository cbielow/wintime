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

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS 1

#include "NamedPipeClient.h"
#include "Defs.h"

#include <iostream>
#include <sstream>
#include <windows.h>
#include <stdio.h>
#include <WinBase.h> // for SetEnvironmentVariable and CreateNamedPipe

namespace WinTime
{

  void NamedPipeClient::sendData(const char* what, int size)
  {
    const auto env_pipe = getenv(s_env_pipename);
    if (!env_pipe)
    {
      std::cerr << "Client: Environment variable '" << s_env_pipename << "' not found! Fix the calling app!\n";
      return;
    }

    HANDLE hPipe;
    hPipe = CreateFileA(env_pipe,
      GENERIC_WRITE,
      FILE_SHARE_WRITE,
      NULL,
      OPEN_EXISTING,
      0,
      NULL);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
      std::cerr << "Client: Could not connect to named pipe!" << env_pipe << '\n';
      return;
    }

    DWORD written_bytes;
    WriteFile(hPipe,
      what,
      size,
      &written_bytes,
      NULL);

    CloseHandle(hPipe);
  }

} // namespace
