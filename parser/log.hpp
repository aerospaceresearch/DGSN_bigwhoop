#pragma once

#include <cstdarg>
#include <cstdio>

class log
{
  public:
    enum { all=0, verbose, debug, info, warning, error, fatal, none };
    static void write(int loglevel, const char *format, ...) noexcept;
    static void setLogLevel(int loglevel) noexcept;

  protected:
    static void checkInit() noexcept;
    static void init() noexcept;

  private:
    log() = delete;
    ~log() = delete;
    log& operator= (const log&) = delete;
    log& operator= (const log&&) = delete;

    static bool initialised_;
    static int  loglevel_;
};

