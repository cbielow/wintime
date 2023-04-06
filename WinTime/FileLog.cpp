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

#include "FileLog.h"

#include "Memory.h"
#include "Process.h"

#include <io.h> // for _chsize_s()
#include <chrono>
#include <iostream>
#include <filesystem>
#include <thread>

using namespace std;

namespace WinTime
{
  /// Try to open (overwrite or append) and lock the file. Returns true on success.


  /// Pass filename and mode; No operation on the file is attempted at this point. Use tryLock() to open and lock the file.

  LockedFile::LockedFile(const std::string& filename, const OpenMode mode)
    : filename_(filename),
      openmode_(mode)
  {
    // First, make sure the file exists:
    auto wfile = widen(filename_);
    if (!std::filesystem::exists(wfile))
    {
      std::wofstream of(wfile);
      if (!of.is_open())
      { // this may print bogus characters if non-ascii letters are printed, but
        // printing to wcerr << wfile will not show non-ascii either, unless fiddling with 
        // _setmode, which breaks ascii output... well done Microsoft!
        std::cerr << "Could not open file " << filename_ << ". Do you have permission to create it in this directory?\n";
        throw std::runtime_error("Could not open file.");
      }
    }
  }

  bool LockedFile::tryLock()
  {
    // at this point, the file exists!  -- see C'tor

    // open file for reading and writing (so we can move to the end, depending on 'mode_'.
    // We cannot query for filesize before, because another process might write stuff to the file
    // immediately afterwards (before we _fsopen for just writing in case we found the file being empty; this 
    // would overwrite the other process' data)
    is_locked_ = ((stream_ = _wfsopen(&widen(filename_)[0], L"r+", _SH_DENYWR)) != NULL);

    if (!is_locked_) return false;

    // depending on mode_, seek to the end, before writing (i.e. append)
    if (openmode_ == OpenMode::OVERWRITE)
    {
      fseek(stream_, 0, SEEK_SET); // beginning of file
    }
    else
    {
      fseek(stream_, 0, SEEK_END); // end of file (append)
    }
    return is_locked_;
  }

   bool LockedFile::isFileEmpty() const
  {
    if (!is_locked_)
    {
      throw std::runtime_error("Trying to write to closed file");
    }
    return (_ftelli64(stream_) == 0);
  }

  /// Write some data to file. @p tryLock() must have been successful before!

  void LockedFile::write(const char* data)
  {
    if (!is_locked_)
    {
      throw std::runtime_error("Trying to write to closed file");
    }
    fprintf(stream_, data);
  }

  /// Truncates the file to the last write position and closes the stream

  LockedFile::~LockedFile()
  {
    if (is_locked_)
    {
      if (_chsize_s(_fileno(stream_), _ftelli64(stream_))) // truncate file to end of last write
      {
        DWORD error = ::GetLastError();
        std::string message = std::system_category().message(error);
        std::cerr << message << '\n';
      }
      fclose(stream_);
    }
  }
  FileLog::FileLog(const std::string& filename, const OpenMode mode, const char sep)
    : filename_(filename),
      mode_(mode),
      sep_(sep)
  {
  }
  void FileLog::log(const std::string& cmd, const PTime& time, const ClientProcessMemoryCounter& pmc)
  {
    // its a bit inefficient to do this here, but we want write access to the file 
    // to be as short as possible
    LockedFile lockf(filename_, mode_);

    // try to lock the file
    while (!lockf.tryLock())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };

    std::stringstream content;
    if (lockf.isFileEmpty())
    { // at start of file .. write header
      printLineToStream(content, '\t', [](auto type, const char sep) { return type.printHeader(sep); }, Command(), time, pmc);
    }
    printLineToStream(content, '\t', [](auto type, const char sep) { return type.print(sep); }, Command{ cmd }, time, pmc);
    lockf.write(content.str().c_str());
  }
} // namespace