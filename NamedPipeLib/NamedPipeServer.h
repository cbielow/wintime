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

#include <string>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h> // for HANDLE; <windef.h> alone is not sufficient

namespace WinTime
{
  /**
    @brief Create named pipe and listen for incoming data from a client.

    Reading the data is still valid even if the client does not exist anymore, but has send data before it got destroyed.

  */
  class NamedPipeServer
  {
    HANDLE hPipe_{  }; ///< handle to the named pipe

    /// establish the pipe and set the environment variable (see Defs.h)
    bool createPipe_();

    /// Create a pipe like '\\.\pipe\Pipe1234', where 1234 is the current ProcessID
    std::string getPipeName_() const;

  public:
    /// calls createPipe_()
    NamedPipeServer();

    /// Disconnects the named pipe
    ~NamedPipeServer();

    /**
    * @brief Retrieve data send by the client
    *
    * @throw std::runtime_error if pipe is invalid
    */
    bool readFromPipe(char* buffer, int size, DWORD& read_bytes);
  };

} // namespace