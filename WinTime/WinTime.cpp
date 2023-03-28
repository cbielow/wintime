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
#include "FileLog.h"
#include "Memory.h"
#include "Process.h"
#include "Time.h"

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
    ClientProcessMemoryCounter pmc;
    DWORD exit_code{1};
  };

  void PrintError(std::string lpszFunction)
  {
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
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

    std::cerr << lpszFunction << " failed with error :" << std::to_string(dw) << ' ' << std::string((char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
  }

  std::optional<TargetInfo> runExternalProcess(const std::string& target_path, std::string& p_command_args)
  {
    std::optional<TargetInfo> result;
    Process process(target_path, p_command_args);
    if (!process.wasCreated())
    {
      PrintError("CreateProcess");
      return result;
    }

    // wait for the child process to finish
    process.waitForFinish();

    DWORD exit_code{ 1 };
    if (!GetExitCodeProcess(process.getPI().hProcess, &exit_code))
    {
      PrintError("Could not get return code of target process");
    }

    ClientProcessMemoryCounter pmc(process.getPI().hProcess);
    auto timings = getProcessTime(process.getPI().hProcess);

    return TargetInfo{ timings, pmc, exit_code };
  }
} // namespace

using namespace WinTime;

int wmain(int argc, wchar_t** argv_wide)
{
  std::vector<const char*> argv(argc);
  std::vector<std::string> argv_data(argc);
  for (int i = 0; i < argc; ++i)
  {
    argv_data[i] = narrow(argv_wide[i]);
    argv[i] = argv_data[i].c_str();
  }
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
    p_parser.ParseCLI(argc, &argv.front());
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

  try
  {
    std::string command = Process::searchPATH(args::get(p_command), p_verbose);

    const auto self_dir = Process::getPathToCurrentProcess();

    const auto arch_result = checkMatchingArch(&command[0]);
    if (p_verbose || arch_result != ArchMatched::SAME)
    {
      std::cout << getArchMatchedExplanation(arch_result, &command[0]) << '\n';
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
      std::string wintime_other = self_dir + "\\" + std::string(WINTIME_EXE_OTHERARCH);
      if (!std::filesystem::exists(wintime_other))
      {
        std::cerr << "Cannot find '" << wintime_other << "'; Please make sure it is present or invoke it manually!\n";
        exit(1);
      }
    
      // it exists... now invoke it and quit this process
      Process process(wintime_other, Process::concatArguments(wintime_other, argc - 1, (&argv[0]) + 1), NORMAL_PRIORITY_CLASS);
      if (!process.wasCreated())
      {
        exit(1);
      }
      process.waitForFinish();
      exit(0);
    }

    std::string wcommand_args = Process::concatArguments(args::get(p_command), StringList(args::get(p_command_args)));
  
    if (p_verbose)
    {
      std::wcerr << "CMD {ARGS}:\n  " << widen(wcommand_args) << '\n';
    }

    
    auto external_process_result = runExternalProcess(command.c_str(), wcommand_args);
    if (!external_process_result)
    {
      std::cerr << "Running external process failed. Aborting.\n";
      return 1;
    }

    // only print to commandline if verbose or not writing to file
    if (!p_output_file || p_verbose)
    {
      external_process_result->pmc.print();
      external_process_result->ptime.print();
    }

    if (p_output_file)
    {
      FileLog fl(p_output_file.Get(), p_append.Get() ? OpenMode::APPEND : OpenMode::OVERWRITE);
      fl.log(wcommand_args, external_process_result->ptime, external_process_result->pmc);
    }

    // return the same exit code as target process
    return external_process_result->exit_code;
  }
  catch (std::exception& ex)
  {
    std::cerr << "Exception occured: " << ex.what() << "\nAborting...\n";
    return 1;
  }

}
