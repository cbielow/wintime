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
#include <fstream>


#include "Time.h"
#include <Memory.h>


namespace WinTime
{
  enum class OpenMode
  {
    OVERWRITE,
    APPEND
  };
  
  /**
  * @brief Print a list of printable objects to a stream as a single line
  * 
  * E.g. each object contributes some cells of a table row. They will be appended in order and separated by @p sep.
  * @p Sep is used to separate between single cells within objects and between cell blocks between objects.
  * 
  * @tparam Lambda A lambda function which accepts a printable object and a separator and creates a partial string
  * @tparam Arg1 The first printable object
  * @tparam Args More printable objects
  * 
  * @param out The stream to print the line to
  * @param sep The separator between elements
  * @param func The lambda function
  * @param first First printable object
  * @param args More printable objects
  */
  template<typename Lambda, typename Arg1, typename ... Args>
  void printLineToStream(std::ostream& out, const char sep, Lambda func, Arg1&& first, Args&&... args)
  {
    out << func(first, sep);
    ((out << sep << func(args, sep)), ...);
    out << '\n';
  }

  /// printable wrapper for a command line
  struct Command
  {
    std::string cmd;
    std::string print(const char /*separator*/) const
    {
      return cmd;
    }

    static std::string printHeader(const char /*separator*/)
    {
      return "cmd";
    }
  };


  /// A locked file with exclusive system-wide write access to a file
  /// Note: Opening a C++ std::ofstream on that file does not work anymore
  class LockedFile
  {
    FILE* stream_ = nullptr;
    std::string filename_;
    OpenMode openmode_{};
    bool is_locked_{ false };
  public:

    /// Pass filename and mode; No operation on the file is attempted at this point. Use tryLock() to open and lock the file.
    LockedFile(const std::string& filename, const OpenMode mode = OpenMode::OVERWRITE);

    /// Try to open (overwrite or append) and lock the file. Returns true on success.
    bool tryLock();

    /// Use this function only right after a successful! tryLock(), not after write()!
    /// Returns true if the file was just created/'appended to empty'/overwritten, and false if appending to non-empty file
    bool isFileEmpty() const;

    /// Write some data to file. @p tryLock() must have been successful before!
    void write(const char* data);

    /// Truncates the file to the last write position and closes the stream
    ~LockedFile();
  };

  class FileLog
  {
  public:

    FileLog(const std::string& filename, const OpenMode mode = OpenMode::OVERWRITE, const char sep = '\t');

    void log(const std::string& cmd, const PTime& time, const ClientProcessMemoryCounter& pmc);

  private:
    const std::string filename_;
    const OpenMode mode_;
    const char sep_;
  };

} // namespace