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

#include <iostream>

#include <locale>
#include <codecvt>
#include <filesystem>
#include <string>
#include <strsafe.h>

#include "Arch.h"
#include "config.h"
#include "Time.h"
#include "FileLog.h"
#include "Process.h"
// from NamedPipeLib
#include <Defs.h>
#include <Memory.h>
#include <NamedPipeServer.h>

#include "args.hxx"  // arg parser

/*
* // at some point we may want to emulate /usr/bin/time on Linux
* 
Usage: /usr/bin/time [OPTIONS] COMMAND [ARG]...
Run COMMAND, then print system resource usage.

  -a, --append              with -o FILE, append instead of overwriting
  -f, --format=FORMAT       use the specified FORMAT instead of the default
  -o, --output=FILE         write to FILE instead of STDERR
  -p, --portability         print POSIX standard 1003.2 conformant string:
                              real %%e
                              user %%U
                              sys %%S
  -q, --quiet               do not print information about abnormal program
                            termination (non-zero exit codes or signals)
  -v, --verbose             print all resource usage information instead of
                            the default format
  -h,  --help               display this help and exit
  -V,  --version            output version information and exit

Commonly usaged format sequences for -f/--format:
(see documentation for full list)
  %%   a literal '%'
  %C   command line and arguments
  %c   involuntary context switches
  %E   elapsed real time (wall clock) in [hour:]min:sec
  %e   elapsed real time (wall clock) in seconds
  %F   major page faults
  %M   maximum resident set size in KB
  %P   percent of CPU this job got
  %R   minor page faults
  %S   system (kernel) time in seconds
  %U   user time in seconds
  %w   voluntary context switches
  %x   exit status of command

*/


namespace WinTime
{
  using StringList = std::vector<std::string>;

  struct TargetInfo
  {
    PTime ptime{};
    DWORD exit_code{1};
  };

  void PrintError(LPTSTR lpszFunction)
  {
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dw,
      MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
      (LPTSTR)&lpMsgBuf,
      0, NULL);

    // Display the error message

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
      LocalSize(lpDisplayBuf) / sizeof(TCHAR),
      TEXT("%s failed with error %d: %s"),
      lpszFunction, dw, lpMsgBuf);

    wprintf(L"%s", (wchar_t*)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
  }

  std::optional<TargetInfo> InjectDll(__in LPCWSTR lpcwszDll, const std::wstring& target_path, std::wstring& p_command_args)
  {
    std::optional<TargetInfo> result;
    Process process(target_path, p_command_args, CREATE_SUSPENDED, true);
    if (!process.wasCreated())
    {
      PrintError(TEXT("CreateProcess"));
      return result;
    }

    LPVOID lpLoadLibraryW = GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), "LoadLibraryW");

    if (!lpLoadLibraryW)
    {
      PrintError(TEXT("GetProcAddress"));
      return result;
    }

    SIZE_T nLength = wcslen(lpcwszDll) * sizeof(WCHAR);

    // allocate mem for dll name
    LPVOID lpRemoteString = VirtualAllocEx(process.getPI().hProcess, NULL, nLength + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!lpRemoteString)
    {
      PrintError(TEXT("VirtualAllocEx"));
      return result;
    }

    // write dll name
    if (!WriteProcessMemory(process.getPI().hProcess, lpRemoteString, lpcwszDll, nLength, NULL)) {

      PrintError(TEXT("WriteProcessMemory"));
      // free allocated memory
      VirtualFreeEx(process.getPI().hProcess, lpRemoteString, 0, MEM_RELEASE);

      return result;
    }

    // call loadlibraryw
    HANDLE hThread = CreateRemoteThread(process.getPI().hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryW, lpRemoteString, NULL, NULL);
  
    if (!hThread) {
      PrintError(TEXT("CreateRemoteThread"));
    }
    else {
      WaitForSingleObject(hThread, 4000);

      //resume suspended process
      ResumeThread(process.getPI().hThread);
    }

    //  free allocated memory
    VirtualFreeEx(process.getPI().hProcess, lpRemoteString, 0, MEM_RELEASE);

    // wait for the child process to finish
    process.waitForFinish();

    DWORD exit_code{ 1 };
    if (!GetExitCodeProcess(process.getPI().hProcess, &exit_code))
    {
      PrintError(TEXT("Could not get return code of target process"));
    }

    auto timings = getProcessTime(process.getPI().hProcess);

    return TargetInfo{ timings, exit_code };
  }
} // namespace

using namespace WinTime;

int main(int argc, char** argv)
{
  args::ArgumentParser p_parser("WinTime - measure time and memory usage of a process.", "");
  p_parser.helpParams.width = 134;
  //args::ValueFlag<int> integer(parser, "integer", "The integer flag", { 'i' });
  //args::ValueFlagList<char> characters(parser, "characters", "The character flag", { 'c' });
  //args::Positional<std::string> foo(parser, "foo", "The foo position");
  args::Flag p_append(p_parser, "append_file", "with -o FILE, append instead of overwriting", { 'a', "append" });
  args::ValueFlag<std::string> p_output_file(p_parser, "output", "write to FILE instead of STDERR", { 'o', "output" });
  args::Flag p_verbose(p_parser, "verbose", "print COMMAND and ARGS", { 'v', "verbose" });
  args::HelpFlag p_help(p_parser, "help", "display this help and exit", { 'h', "help" });
  args::Group group(p_parser, "", args::Group::Validators::AtLeastOne);
  args::Flag p_version(group, "version", "output version information and exit", { 'V', "version" });
  args::Positional<std::string> p_command(group, "COMMAND", "the executable to run");
  args::PositionalList<std::string> p_command_args(p_parser, "ARG", "arguments to COMMAND");
  try
  {
    p_parser.ParseCLI(argc, argv);
  }
  catch (args::Help)
  {
    std::cout << p_parser;
    return 0;
  }
  catch (args::ParseError e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << p_parser;
    return 1;
  }
  catch (args::ValidationError e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << p_parser;
    return 1;
  }

  if (p_version)
  { // TODO Use CMake to configure a header
    std::cerr << "Version: " << CMAKE_PROJECT_VERSION << '\n';
    exit(0);
  }

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wcommand = converter.from_bytes(args::get(p_command));


  const auto self_dir = Process::getPathToCurrentProcess();

  const auto arch_result = checkMatchingArch(&wcommand[0]);
  if (p_verbose || arch_result != ArchMatched::SAME)
  {
    std::wcout << getArchMatchedExplanation(arch_result, &wcommand[0]) << '\n';
  }
  if (arch_result == ArchMatched::TARGET_UNKNOWN)
  {
    std::cerr << "Cannot determine target architecture. Is it an executable?\n";
    exit(1);
  }  
  // try auto switching to WINTIME_EXE_OTHERARCH
  if (arch_result == ArchMatched::MIXED)
  {
    std::cerr << "Trying to find '" << WINTIME_EXE_OTHERARCH << "' automatically...\n";
    std::wstring wintime_other = self_dir + L"\\" + converter.from_bytes(std::string(WINTIME_EXE_OTHERARCH));
    if (!std::filesystem::exists(wintime_other))
    {
      std::wcerr << "Cannot find '" << wintime_other << "'; Please make sure it is present or invoke it manually!\n";
      exit(1);
    }
    
    // it exists... now invoke it and quit this process
    Process process(wintime_other, Process::concatArguments(converter.to_bytes(wintime_other), argc - 1, argv + 1), NORMAL_PRIORITY_CLASS, false);
    if (!process.wasCreated())
    {
      exit(1);
    }
    process.waitForFinish();
    exit(0);
  }

  std::wstring wcommand_args = Process::concatArguments(args::get(p_command), StringList(args::get(p_command_args)));
  
  if (p_verbose)
  {
    std::wcerr << "CMD {ARGS}:\n  " << (wcommand_args) << '\n';
  }

  std::wstring dll_path = self_dir + L"\\" + converter.from_bytes(std::string(WINTIME_DLL));

  if (!std::filesystem::exists(dll_path))
  {
    std::wcerr << "Could not find DLL '" << (dll_path) << "' for injection. Make sure it is present!\n";
    return(1);
  }

  // create pipe before injecting DLL
  NamedPipeServer ps;

  auto inject_result = InjectDll(dll_path.c_str(), wcommand.c_str(), wcommand_args);
  if (!inject_result)
  {
    std::cerr << "Injecting Dll failed. Aborting.\n";
    return 1;
  }

  char buffer[sizeof(ClientProcessMemoryCounter)];
  DWORD read_bytes;
  if (!ps.readFromPipe(buffer, sizeof(buffer), read_bytes))
  {
    std::cerr << "Could not read client data from pipe!\n";
  }
  ClientProcessMemoryCounter pmc(buffer);
  pmc.print();

  inject_result->ptime.print();

  if (p_output_file)
  {
    FileLog fl(p_output_file.Get(), p_append.Get() ? OpenMode::APPEND : OpenMode::OVERWRITE);
    fl.log(converter.to_bytes(wcommand_args), inject_result->ptime, pmc);
  }

  // return the same exit code as target process
  return inject_result->exit_code;
}
