
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

#include "NamedPipeServer.h"

#include "Defs.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <sstream>

#include <stdio.h>

namespace WinTime
{
  NamedPipeServer::NamedPipeServer()
  {
    createPipe_();
  }

  NamedPipeServer::~NamedPipeServer()
  {
    DisconnectNamedPipe(hPipe_);
  }

  bool NamedPipeServer::readFromPipe(char* buffer, int size, DWORD& read_bytes)
  {
    if (hPipe_ == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("Named pipe was not established. Make sure createPipe() was successful before calling 'readFromPipe()'.\n");
    }
    return ReadFile(hPipe_, buffer, size, &read_bytes, NULL);
  }

  bool NamedPipeServer::createPipe_()
  {
    /// create a named pipe (using the processID of this process as identifier)
    const auto pipe_name = getPipeName_();
    /// ... and use an ENV, so the child process knows the name of the pipe
    SetEnvironmentVariableA(s_env_pipename, &pipe_name[0]);
    hPipe_ = CreateNamedPipeA(&pipe_name[0],
      PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
      PIPE_TYPE_BYTE | PIPE_REJECT_REMOTE_CLIENTS,
      1, // nMaxInstances
      1024 * 16,
      1024 * 16,
      NMPWAIT_USE_DEFAULT_WAIT,
      NULL);
    if (hPipe_ == INVALID_HANDLE_VALUE)
    {
      std::cerr << "could not create named pipe to communicate with target process.\n";
      return false;
    }
    return true;
  }

  std::string NamedPipeServer::getPipeName_() const
  {
    std::stringstream procID;
    procID << GetCurrentProcessId();
    std::string name = "\\\\.\\pipe\\Pipe";
    name += procID.str();
    return name;
  }

} // namespace
