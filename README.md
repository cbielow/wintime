# WinTime ~ a reimplementation of /usr/bin/time for Windows

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Ever needed to measure the execution time and/or maximal RAM usage of a program on Microsoft Windows?
Existing solutions like TaskManager, or [ProcessExplorer](https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer) are not scriptable, and once the process finishes, the data is gone.
`wmic process list` comes close, but is only a snapshot in time, i.e. you'd need to run it very often and might still miss the peak RAM usage of a process.

So how do you reliably measure peak RAM usage of, for example, the Microsoft C++ compiler?!
Answer: use WinTime :-)

## Usage

```
  WinTime64.exe {OPTIONS} [COMMAND] [ARG...]

    WinTime - measure time and memory usage of a process.

  OPTIONS:

      -a, --append                      with -o FILE, append instead of overwriting
      -o[output], --output=[output]     write to FILE instead of STDERR
      -v, --verbose                     print COMMAND and ARGS
      -h, --help                        display this help and exit
      -V, --version                     output version information and exit
      COMMAND                           the executable to run
      ARG...                            arguments to COMMAND
      "--" can be used to terminate flag options and force all following arguments to be treated as positional options
```

## Example

Measure peak RAM and CPU/wall/user time of MS Excel
```
WinTime64 "C:\Program Files\Microsoft Office\root\Office16\EXCEL.EXE"
```


The output on the command line might look something like this:
```
PageFaultCount: 40224
PeakWorkingSetSize: 134.2 MiB
QuotaPeakPagedPoolUsage: 1.685 MiB
QuotaPeakNonPagedPoolUsage: 89.02 KiB
PeakPagefileUsage: 120.1 MiB
Creation time 2023/02/13 10:19:04.200
    Exit time 2023/02/13 10:19:09.362
    Wall time:  0 days, 00:00:05.161 (5.16 seconds)
    User time:  0 days, 00:00:00.484 (0.48 seconds)
  Kernel time:  0 days, 00:00:00.343 (0.34 seconds)
```


Same for Firefox, but with additional logging to a `log.txt` file (append mode)
```
WinTime64 -a -o log.txt -- Firefox.exe
```

When passing additional arguments to either WinTime or your executable, it is probably best to use the `--` separator commonly found on Linux. This ensures that arguments to WinTime vs. your executable are clearly distinct.
Imagine your program has a `-v` flag (WinTime also has it). Calling
```
WinTime64 yourProg.exe -v
```
will actually assign the `-v` to WinTime, and not pass it on to `yourProg.exe`.
To make sure all arguments after `yourProg.exe` are actually forwarded to yourProg.exe, use the `--` separator:
```
WinTime64 [wintime_options] -- yourProg.exe -v
```
where `[wintime_options]` is the place for optional arguments to WinTime itself.



## Features

 - peak **RAM** usage
 - total **CPU time** (wall, kernel, user) with high resolution
 - PageFaultCount
 - PeakPagefileUsage
 - log file output
 - similar command line interface as /usr/bin/time

## Installation

##### Precompiled Binary

The easiest is probably to use the precompiled executables `WinTime32.exe` and `WinTime64.exe`, see [Releases](https://github.com/cbielow/wintime/releases).
If you have 64-bit targets (=executables you want to analyse), you need to run the 64-bit version, and vice versa for 32-bit targets.
If do not know the architecture of your target, simply invoke any of `WinTime32.exe` or `WinTime64.exe` -- WinTime will automatically switch to the correct architecture depending on the target.

##### Compile yourself

Alternatively, you can compile WinTime by cloning this repo, running CMake and build the project.
It builds fine using Visual Studio 2022. Other versions of Visual Studio have not been tested!
We will build the 32-bit version, as well as the 64-bit version here.
Note that it is currently impossible to have both architectures in a single Visual Studio Solution file when using CMake.
Hence, we use two build trees:

```
git clone https://github.com/cbielow/wintime.git

mkdir wintime_build32
cd wintime_build32
cmake -G "Visual Studio 17 2022"  -A Win32 ..\wintime
msbuild -p:Configuration=Release ALL_BUILD.vcxproj
cd ..

mkdir wintime_build64
cd wintime_build64
cmake -G "Visual Studio 17 2022"  -A x64 ..\wintime
msbuild -p:Configuration=Release ALL_BUILD.vcxproj

```

This will create an executable in `wintime_build32\Release\WinTime32.exe` and `wintime_build64\Release\WinTime64.exe`
If you have 64-bit targets (=executables you want to analyse), you need to run the 64-bit version, and vice versa for 32-bit targets.
If do not know the architecture of your target, simply copy the `WinTime32.exe`, `WinTime64.exe` (and the accompanying `WinTimeDll32.dll` and `WinTimeDll64.dll`) into a single folder.

```
cd ..
copy wintime_build64\WinTime\Release\WinTime* wintime_build32\WinTime\Release
```

This way, WinTime will automatically switch to the correct architecture depending on the target.

## Development

Want to contribute? Great!
Open a [bug report, feature request](https://github.com/cbielow/wintime/issues) or [pull request](https://github.com/cbielow/wintime/pull).

## Technical details

WinTime makes heavy use of Windows API functions. It works by launching the target process and immediatedly injecting a Dll into it. Upon termination, the Dll will get notified that the target process is about to finish, and take the opportunity to measure peak ram usage and other metrics. This data will be send to WinTime using named pipes. WinTime itself will measure the CPU time of the target process, which can be done even after the process has exited iff you know the process ID (which we do since we spawned the process).
The `-o` option allows to write/append the data to a log, which requires some file locking magic to ensure that concurrent access to the log does not mangle its content.

## How accurate is the data?

##### RAM
The RAM usage of the target process is slightly increased by the injected Dll and the Windows functions it calls -- about 1.3 MiB. Similarly, PageFaultCount increases by about 344 (which makes sense for a 4kb page, i.e. 344*4 Kb = 1376 Kb). Those are worst case numbers. If the target process natively calls GetProcessMemoryInfo(), these numbers will go down (due to reused pages). At a bare minimum, the RAM usage will increase by the size of the injected Dll, which is currently ~50 Kb.

##### CPU
The CPU time (wall time, kernel time, user time) are high resolution, but include some minor overhead of injecting the Dll and time of calling the RAM usage function inside the target process -- on my system that is at most 5 milliseconds (this is how long a process executes which does absolutely nothing except execution of an empty `main()` function plus the overhead of the injected WinTime.dll). 

## License
MIT

**Free Software, Hell Yeah!**

## Acknowledgements
We use C++[args](https://github.com/Taywee/args) to parse command line options.
