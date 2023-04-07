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
  
  
  int countBackslashesAtEnd(const std::string& arg);

  void substitute(std::string& str, const std::string& search,
    const std::string& replace);


  class Process
  {
  public:

    static std::string getPathToCurrentProcess();

    static std::string concatArguments(const std::string& exe, int more_args_argc, const char** more_args_argv);

    static std::string concatArguments(const std::string& exe, std::vector<std::string> command_args);

    /// search for an executable on the %PATH% environment variable.
    static std::string searchPATH(const std::string& exe, bool verbose = false);

    /// Starts a process, with extra arguments (if not empty)
    /// The @p target_exe must be an absolute or relative path. The %PATH% environment variable is not used! (use @p searchPATH if you need that)
    Process(const std::string& target_exe, const std::string& p_command_args, DWORD dwCreationFlags = 0);

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

  std::string narrow(const std::wstring& wide_str);

  std::wstring widen(const std::string& uft8_str);


} // namespace