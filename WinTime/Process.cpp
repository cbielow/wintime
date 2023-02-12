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
#define UNICODE 1
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1
 // Windows Header Files:
#include <windows.h>

#include <codecvt>
#pragma comment (lib, "Shlwapi.lib")
#include <Shlwapi.h>   // for PathRemoveFileSpec

#include "Process.h"

using namespace std;

namespace WinTime
{

  int countBackslashesAtEnd(const std::wstring& arg)
  {
    int result{ 0 };
    for (auto rit = arg.rbegin(); rit != arg.rend(); ++rit)
    {
      if (*rit != '\\') break;
      ++result;
    }
    return result;
  }

  void substitute(std::wstring& str, const std::wstring& search,
    const std::wstring& replace) {
    size_t pos = 0;
    while ((pos = str.find(search, pos)) != std::string::npos) {
      str.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }

  std::wstring Process::getPathToCurrentProcess()
  {
    wchar_t selfdir[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, selfdir, MAX_PATH);
    PathRemoveFileSpec(selfdir);
    return std::wstring(selfdir);
  }

  std::wstring Process::concatArguments(const std::string& exe, int more_args_argc, char** more_args_argv)
  {
    std::vector<std::string> tmp;
    for (int i = 0; i < more_args_argc; ++i)
      tmp.push_back(more_args_argv[i]);
    return concatArguments(exe, tmp);
  }

  std::wstring Process::concatArguments(const std::string& exe, std::vector<std::string> command_args)
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    command_args.insert(command_args.begin(), exe); // prepend the exe
    std::wstring result;
    for (auto arga : command_args)
    {
      std::wstring arg = converter.from_bytes(arga);
      // Does arg have quotes? E.g. '-DQT_TESTCASE_BUILDDIR="C:/dev/openms_build_ninja"'
      if (arg.find('"') != std::string::npos)
      { // escape them, otherwise the commandline parser will wrongly interpret those quotes
        substitute(arg, L"\"", L"\\\"");
      }

      if (arg.find(' ') != std::string::npos)
      { // only quote if required
        result += '"';
        result += arg;
        // careful when adding a quote at the end, since 'c:\somepath\' will become 'c:\somepath\"', i.e. the closing quote is escaped and the argument is not closed
        if (countBackslashesAtEnd(arg) % 2 == 1)
        { // add extra backslash
          result += '\\';
        }
        result += '"';
      }
      else
      {
        result += arg;
      }
      result += ' ';
    }
    return result;
  }

  Process::Process(const std::wstring& target_exe, std::wstring& p_command_args, DWORD dwCreationFlags, bool search_path)
  {
    process_information_ = new PROCESS_INFORMATION;
    STARTUPINFO             startupInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(STARTUPINFO);

    // CreateProcess does not use the search path
    // We must do that manually
    const WCHAR* target_used = &target_exe[0];
    TCHAR buffer[1000];
    if (search_path)
    {
      SearchPath(NULL, &target_exe[0], TEXT(".exe"), sizeof(buffer) / sizeof(TCHAR), buffer, NULL);
      target_used = buffer;
    }

    was_created_ = CreateProcess(target_used, &p_command_args[0], NULL, NULL, FALSE,
      dwCreationFlags, NULL, NULL, &startupInfo, process_information_);
  }

  bool Process::wasCreated() const
  {
    return was_created_;
  }

  PROCESS_INFORMATION& Process::getPI()
  {
    return *process_information_;
  }

  void Process::waitForFinish()
  {
    // wait for the child process to finish
    WaitForSingleObject(process_information_->hProcess, INFINITE);
  }

  Process::~Process()
  {
    CloseHandle(process_information_->hProcess);
    delete process_information_;
  }

} // namespace
