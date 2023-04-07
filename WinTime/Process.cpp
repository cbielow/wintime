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
#include <filesystem>

#pragma comment (lib, "Shlwapi.lib")
#include <Shlwapi.h>   // for PathRemoveFileSpec

#include "Process.h"
#include <iostream>

using namespace std;

namespace WinTime
{

  int countBackslashesAtEnd(const std::string& arg)
  {
    int result{ 0 };
    for (auto rit = arg.rbegin(); rit != arg.rend(); ++rit)
    {
      if (*rit != '\\') break;
      ++result;
    }
    return result;
  }

  void substitute(std::string& str, const std::string& search,
    const std::string& replace) {
    size_t pos = 0;
    while ((pos = str.find(search, pos)) != std::string::npos) {
      str.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }

  std::string narrow(const std::wstring& wide_str)
  {
    std::string buffer(wide_str.size() * 2 + 2, '\0');
    auto bytes_written = WideCharToMultiByte(CP_UTF8,
      0,
      wide_str.c_str(),
      -1,
      buffer.data(), 1000,
      NULL, NULL);
    buffer.resize(bytes_written);
    return buffer;
  }

  std::wstring widen(const std::string& uft8_str)
  {
    std::wstring buffer(uft8_str.size() + 2, '\0');
    auto bytes_written = MultiByteToWideChar(CP_UTF8,
      0,
      uft8_str.c_str(),
      -1,
      buffer.data(), buffer.size());
    buffer.resize(bytes_written);
    return buffer;
  }

  std::string Process::getPathToCurrentProcess()
  {
    wchar_t selfdir[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, selfdir, MAX_PATH);
    PathRemoveFileSpecW(selfdir);
    return narrow(selfdir);
  }

  std::string Process::concatArguments(const std::string& exe, int more_args_argc, const char** more_args_argv)
  {
    std::vector<std::string> tmp;
    for (int i = 0; i < more_args_argc; ++i)
      tmp.push_back(more_args_argv[i]);
    return concatArguments(exe, tmp);
  }

  std::string Process::concatArguments(const std::string& exe, std::vector<std::string> command_args)
  {
    //std::string_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    command_args.insert(command_args.begin(), exe); // prepend the exe
    std::string result;
    for (auto arg : command_args)
    {
      // Does arg have quotes? E.g. '-DQT_TESTCASE_BUILDDIR="C:/dev/openms_build_ninja"'
      if (arg.find('"') != std::string::npos)
      { // escape them, otherwise the commandline parser will wrongly interpret those quotes
        substitute(arg, "\"", "\\\"");
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

  std::string Process::searchPATH(const std::string& target_exe, bool verbose)
  {
    auto wt = widen(target_exe);
    if (!std::filesystem::exists(wt))
    {
      const int BUF_SIZE = 3000;
      wchar_t buffer[BUF_SIZE];
      auto ret = SearchPathW(NULL, &wt[0], (L".exe"), sizeof(buffer) / sizeof(wchar_t), buffer, NULL);
      if (!ret)
      {
        throw std::runtime_error("Could not find executable '" + target_exe + "' (%PATH% was also checked).");
      }
      if (ret > BUF_SIZE)
      {
        throw std::runtime_error("Path to '" + target_exe + "' is longer than " + std::to_string(BUF_SIZE) + ". Cannot store result. Fix the program or use a shorter path.");
      }
      std::string result = narrow(buffer);
      if (verbose)
      {
        std::wcout << "Found '" << wt << "' in PATH as '" << buffer << "'.\n";
      }
      return result;
    }
    return target_exe;
  }

  Process::Process(const std::string& target_exe, const std::string& p_command_args, DWORD dwCreationFlags)
  {
    process_information_ = new PROCESS_INFORMATION;
    STARTUPINFOW             startupInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    std::string pca = p_command_args;
    auto texe = widen(target_exe);
    auto pargs = widen(pca);
    was_created_ = CreateProcessW(&texe[0], &pargs[0], NULL, NULL, FALSE,
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
