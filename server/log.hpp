#pragma once

#include <cstdio>
#include <cstdarg>

class log
{
  public:
    enum { all=0, verbose, debug, info, warning, error, fatal, none };
    static void write(int nLevel, const char *szFormat, ...) noexcept;
    static void setLevel(int nLevel) noexcept;

  protected:
    static void checkInit() noexcept;
    static void init() noexcept;

  private:
    log() = delete;
    ~log() = delete;
    log& operator= (const log&) = delete;
    log& operator= (const log&&) = delete;

    static bool m_bInitialised;
    static int  m_nLevel;
};

