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
#include <vector>

namespace WinTime
{
  
  
  int countBackslashesAtEnd(const std::wstring& arg);

  void substitute(std::wstring& str, const std::wstring& search,
    const std::wstring& replace);


  class Process
  {
  public:

    static std::wstring getPathToCurrentProcess();

    static std::wstring concatArguments(const std::string& exe, int more_args_argc, char** more_args_argv);

    static std::wstring concatArguments(const std::string& exe, std::vector<std::string> command_args);

    Process(const std::wstring& target_exe, std::wstring& p_command_args, DWORD dwCreationFlags = 0, bool search_path = true);

    /// Was the process succesfully created in the C'tor?
    bool wasCreated() const;

    /// get process information of the internal process
    PROCESS_INFORMATION& getPI();
    
    /// wait for the child process to finish
    void waitForFinish();

    ~Process();

  private:
    PROCESS_INFORMATION* process_information_;
    bool was_created_;
  };

} // namespace